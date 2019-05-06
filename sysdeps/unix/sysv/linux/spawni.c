/* POSIX spawn interface.  Linux version.
   Copyright (C) 2016-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <spawn.h>
#include <paths.h>
#include <dirent.h>
#include <sys/resource.h>
#include <not-cancel.h>
#include <local-setxid.h>
#include <shlib-compat.h>
#include <nptl/pthreadP.h>
#include <ctype.h>
#include <dl-sysdep.h>
#include <libc-pointer-arith.h>
#include <stackmap.h>
#include "spawn_int.h"

/* The Linux implementation of posix_spawn{p} uses the clone syscall directly
   with CLONE_VM and CLONE_VFORK flags and an allocated stack.  The new stack
   and start function solves most the vfork limitation (possible parent
   clobber due stack spilling). The remaining issue are:

   1. That no signal handlers must run in child context, to avoid corrupting
      parent's state.
   2. The parent must ensure child's stack freeing.
   3. Child must synchronize with parent to enforce 2. and to possible
      return execv issues.

   The first issue is solved by blocking all signals in child, even
   the NPTL-internal ones (SIGCANCEL and SIGSETXID).  The second and
   third issue is done by a stack allocation in parent, and by using a
   field in struct spawn_args where the child can write an error
   code. CLONE_VFORK ensures that the parent does not run until the
   child has either exec'ed successfully or exited.  */


/* The Unix standard contains a long explanation of the way to signal
   an error after the fork() was successful.  Since no new wait status
   was wanted there is no way to signal an error using one of the
   available methods.  The committee chose to signal an error by a
   normal program exit with the exit code 127.  */
#define SPAWN_ERROR	127

#ifdef __ia64__
# define CLONE(__fn, __stackbase, __stacksize, __flags, __args) \
  __clone2 (__fn, __stackbase, __stacksize, __flags, __args, 0, 0, 0)
#else
# define CLONE(__fn, __stack, __stacksize, __flags, __args) \
  __clone (__fn, __stack, __flags, __args)
#endif

/* Since ia64 wants the stackbase w/clone2, re-use the grows-up macro.  */
#if _STACK_GROWS_UP || defined (__ia64__)
# define STACK(__stack, __stack_size) (__stack)
#elif _STACK_GROWS_DOWN
# define STACK(__stack, __stack_size) (__stack + __stack_size)
#endif

struct posix_spawn_args
{
  sigset_t oldmask;
  const char *file;
  int (*exec) (const char *, char *const *, char *const *);
  const posix_spawn_file_actions_t *fa;
  const posix_spawnattr_t *restrict attr;
  char *const *argv;
  char *const *envp;
  int xflags;
  int err;
};

/* Close all file descriptor up to FROM by interacting /proc/self/fd.  */
static bool
spawn_closefrom (int from)
{
  struct dirent64 entries[1024 / sizeof (struct dirent64)];

  int dirfd = __open ("/proc/self/fd", O_RDONLY | O_DIRECTORY, 0);
  if (dirfd == -1)
    return false;

  ssize_t r;
  while ((r = __getdents64 (dirfd, entries, sizeof (entries))) > 0)
    {
      struct dirent64 *dp = entries;
      struct dirent64 *edp = (void *)((uintptr_t) dp + r);

      for (struct dirent64 *dp = entries; dp < edp;
	   dp = (void *)((uintptr_t) dp + dp->d_reclen))
	{
	  int fd = 0;

	  if (dp->d_name[0] == '.')
	    continue;

	  for (const char *s = dp->d_name; isdigit (*s); s++)
	    fd = 10 * fd + (*s - '0');

	  if (fd == dirfd || fd < from)
	    continue;

	  __close_nocancel (fd);
	}
    }

  __close_nocancel (dirfd);
  return true;
}

/* Function used in the clone call to setup the signals mask, posix_spawn
   attributes, and file actions.  It run on its own stack (provided by the
   posix_spawn call).  */
static int
spawni_child (void *arguments)
{
  struct posix_spawn_args *args = arguments;
  const posix_spawnattr_t *restrict attr = args->attr;
  const posix_spawn_file_actions_t *file_actions = args->fa;

  /* The child must ensure that no signal handler are enabled because it shared
     memory with parent, so the signal disposition must be either SIG_DFL or
     SIG_IGN.  It does by iterating over all signals and although it could
     possibly be more optimized (by tracking which signal potentially have a
     signal handler), it might requires system specific solutions (since the
     sigset_t data type can be very different on different architectures).  */
  struct sigaction sa;
  memset (&sa, '\0', sizeof (sa));

  sigset_t hset;
  __sigprocmask (SIG_BLOCK, 0, &hset);
  for (int sig = 1; sig < _NSIG; ++sig)
    {
      if ((attr->__flags & POSIX_SPAWN_SETSIGDEF)
	  && __sigismember (&attr->__sd, sig))
	{
	  sa.sa_handler = SIG_DFL;
	}
      else if (__sigismember (&hset, sig))
	{
	  if (__is_internal_signal (sig))
	    sa.sa_handler = SIG_IGN;
	  else
	    {
	      __libc_sigaction (sig, 0, &sa);
	      if (sa.sa_handler == SIG_IGN)
		continue;
	      sa.sa_handler = SIG_DFL;
	    }
	}
      else
	continue;

      __libc_sigaction (sig, &sa, 0);
    }

#ifdef _POSIX_PRIORITY_SCHEDULING
  /* Set the scheduling algorithm and parameters.  */
  if ((attr->__flags & (POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER))
      == POSIX_SPAWN_SETSCHEDPARAM)
    {
      if (__sched_setparam (0, &attr->__sp) == -1)
	goto fail;
    }
  else if ((attr->__flags & POSIX_SPAWN_SETSCHEDULER) != 0)
    {
      if (__sched_setscheduler (0, attr->__policy, &attr->__sp) == -1)
	goto fail;
    }
#endif

  if ((attr->__flags & POSIX_SPAWN_SETSID) != 0
      && __setsid () < 0)
    goto fail;

  /* Set the process group ID.  */
  if ((attr->__flags & POSIX_SPAWN_SETPGROUP) != 0
      && __setpgid (0, attr->__pgrp) != 0)
    goto fail;

  /* Set the effective user and group IDs.  */
  if ((attr->__flags & POSIX_SPAWN_RESETIDS) != 0
      && (local_seteuid (__getuid ()) != 0
	  || local_setegid (__getgid ()) != 0))
    goto fail;

  /* Execute the file actions.  */
  if (file_actions != 0)
    {
      int cnt;
      struct rlimit64 fdlimit;
      bool have_fdlimit = false;

      for (cnt = 0; cnt < file_actions->__used; ++cnt)
	{
	  struct __spawn_action *action = &file_actions->__actions[cnt];

	  switch (action->tag)
	    {
	    case spawn_do_close:
	      if (__close_nocancel (action->action.close_action.fd) != 0)
		{
		  if (!have_fdlimit)
		    {
		      __getrlimit64 (RLIMIT_NOFILE, &fdlimit);
		      have_fdlimit = true;
		    }

		  /* Signal errors only for file descriptors out of range.  */
		  if (action->action.close_action.fd < 0
		      || action->action.close_action.fd >= fdlimit.rlim_cur)
		    goto fail;
		}
	      break;

	    case spawn_do_open:
	      {
		/* POSIX states that if fildes was already an open file descriptor,
		   it shall be closed before the new file is opened.  This avoid
		   pontential issues when posix_spawn plus addopen action is called
		   with the process already at maximum number of file descriptor
		   opened and also for multiple actions on single-open special
		   paths (like /dev/watchdog).  */
		__close_nocancel (action->action.open_action.fd);

		int ret = __open_nocancel (action->action.open_action.path,
					   action->action.
					   open_action.oflag | O_LARGEFILE,
					   action->action.open_action.mode);

		if (ret == -1)
		  goto fail;

		int new_fd = ret;

		/* Make sure the desired file descriptor is used.  */
		if (ret != action->action.open_action.fd)
		  {
		    if (__dup2 (new_fd, action->action.open_action.fd)
			!= action->action.open_action.fd)
		      goto fail;

		    if (__close_nocancel (new_fd) != 0)
		      goto fail;
		  }
	      }
	      break;

	    case spawn_do_dup2:
	      /* Austin Group issue #411 requires adddup2 action with source
		 and destination being equal to remove close-on-exec flag.  */
	      if (action->action.dup2_action.fd
		  == action->action.dup2_action.newfd)
		{
		  int fd = action->action.dup2_action.newfd;
		  int flags = __fcntl (fd, F_GETFD, 0);
		  if (flags == -1)
		    goto fail;
		  if (__fcntl (fd, F_SETFD, flags & ~FD_CLOEXEC) == -1)
		    goto fail;
		}
	      else if (__dup2 (action->action.dup2_action.fd,
			       action->action.dup2_action.newfd)
		       != action->action.dup2_action.newfd)
		goto fail;
	      break;

	    case spawn_do_chdir:
	      if (__chdir (action->action.chdir_action.path) != 0)
		goto fail;
	      break;

	    case spawn_do_fchdir:
	      if (__fchdir (action->action.fchdir_action.fd) != 0)
		goto fail;
	      break;

	    case spawn_do_closefrom:
	      if (!spawn_closefrom (action->action.closefrom_action.from))
		goto fail;
	      break;
	    }
	}
    }

  /* Set the initial signal mask of the child if POSIX_SPAWN_SETSIGMASK
     is set, otherwise restore the previous one.  */
  __sigprocmask (SIG_SETMASK, (attr->__flags & POSIX_SPAWN_SETSIGMASK)
		 ? &attr->__ss : &args->oldmask, 0);

  args->exec (args->file, args->argv, args->envp);

fail:
  /* errno should have an appropriate non-zero value; otherwise,
     there's a bug in glibc or the kernel.  For lack of an error code
     (EINTERNALBUG) describing that, use ECHILD.  Another option would
     be to set args->err to some negative sentinel and have the parent
     abort(), but that seems needlessly harsh.  */
  args->err = errno ? : ECHILD;
  _exit (SPAWN_ERROR);
}

static int
spawni_clone (struct posix_spawn_args *args, void *stack, size_t stack_size,
	      pid_t *pid)
{
  int ec;
  pid_t new_pid;

  /* The clone flags used will create a new child that will run in the same
     memory space (CLONE_VM) and the execution of calling thread will be
     suspend until the child calls execve or _exit.

     Also since the calling thread execution will be suspend, there is not
     need for CLONE_SETTLS.  Although parent and child share the same TLS
     namespace, there will be no concurrent access for TLS variables (errno
     for instance).  */
  new_pid = CLONE (spawni_child, STACK (stack, stack_size), stack_size,
		   CLONE_VM | CLONE_VFORK | SIGCHLD, args);

  /* It needs to collect the case where the auxiliary process was created
     but failed to execute the file (due either any preparation step or
     for execve itself).  */
  if (new_pid > 0)
    {
      /* Also, it handles the unlikely case where the auxiliary process was
	 terminated before calling execve as if it was successfully.  The
	 args.err is set to 0 as default and changed to a positive value
	 only in case of failure, so in case of premature termination
	 due a signal args.err will remain zeroed and it will be up to
	 caller to actually collect it.  */
      ec = args->err;
      if (ec > 0)
	/* There still an unlikely case where the child is cancelled after
	   setting args.err, due to a positive error value.  Also there is
	   possible pid reuse race (where the kernel allocated the same pid
	   to an unrelated process).  Unfortunately due synchronization
	   issues where the kernel might not have the process collected
	   the waitpid below can not use WNOHANG.  */
	__waitpid (new_pid, NULL, 0);
    }
  else
    ec = -new_pid;

  if ((ec == 0) && (pid != NULL))
    *pid = new_pid;

  return ec;
}

#if SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_15)
/* This is compatibility function required to enable posix_spawn run
   script without shebang definition for older posix_spawn versions
   (2.15).  */
static int
execve_compat (const char *filename, char *const argv[], char *const envp[])
{
  __execve (filename, argv, envp);

  if (errno == ENOEXEC)
    {
      char *const *cargv = argv;
      ptrdiff_t argc = 0;
      while (cargv[argc++] != NULL);

      /* Construct an argument list for the shell.  */
      char *new_argv[argc + 2];
      new_argv[0] = (char *) _PATH_BSHELL;
      new_argv[1] = (char *) filename;
      if (argc > 1)
	memcpy (new_argv + 2, argv + 1, argc * sizeof (char *));
      else
	new_argv[2] = NULL;

      /* Execute the shell.  */
      __execve (new_argv[0], new_argv, envp);
    }

  return -1;
}

/* Allocates a stack using mmap to call clone.  The stack size is based on
   number of arguments since it would be used on compat mode which may call
   execvpe/execve_compat.  */
static int
spawnix_compat (struct posix_spawn_args *args, pid_t *pid)
{
  char *const *argv = args->argv;

  /* To avoid imposing hard limits on posix_spawn{p} the total number of
     arguments is first calculated to allocate a mmap to hold all possible
     values.  */
  ptrdiff_t argc = 0;
  /* Linux allows at most max (0x7FFFFFFF, 1/4 stack size) arguments
     to be used in a execve call.  We limit to INT_MAX minus one due the
     compatiblity code that may execute a shell script (maybe_script_execute)
     where it will construct another argument list with an additional
     argument.  */
  ptrdiff_t limit = INT_MAX - 1;
  while (argv[argc++] != NULL)
    if (argc == limit)
      {
	errno = E2BIG;
	return errno;
      }

  size_t argv_size = (argc * sizeof (void *));
  /* We need at least a few pages in case the compiler's stack checking is
     enabled.  In some configs, it is known to use at least 24KiB.  We use
     32KiB to be "safe" from anything the compiler might do.  Besides, the
     extra pages won't actually be allocated unless they get used.
     It also acts the slack for spawn_closefrom (including MIPS64 getdents64
     where it might use about 1k extra stack space.  */
  argv_size += (32 * 1024);

  /* Allocate a stack with an extra guard page.  */
  size_t guard_size = stack_guard_size ();
  size_t stack_size = guard_size + ALIGN_UP (argv_size, __getpagesize ());
  void *stack = stack_allocate (stack_size, guard_size);
  if (__glibc_unlikely (stack == MAP_FAILED))
    return errno;

  int ec = spawni_clone (args, stack, stack_size, pid);

  __munmap (stack, stack_size);

  return ec;
}
#endif

/* For SPAWN_XFLAGS_TRY_SHELL we need to execute a script even without
   a shebang.  To accomplish it we pass as callback to spawni_child
   __execvpe (which call maybe_script_execute for such case) or
   execve_compat (which mimics the semantic using execve).  */
static int
spawn_process (struct posix_spawn_args *args, pid_t *pid)
{
  int ec;

#if SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_15)
  if (args->xflags & SPAWN_XFLAGS_TRY_SHELL)
    {
      args->exec = args->xflags & SPAWN_XFLAGS_USE_PATH
		   ? __execvpe  : execve_compat;
      ec = spawnix_compat (args, pid);
    }
  else
#endif
    {
      args->exec = args->xflags & SPAWN_XFLAGS_USE_PATH
		   ? __execvpex : __execve;

      /* spawni_clone stack usage need to take in consideration spawni_child
	 stack usage and subsequent functions called:

	 - sigprocmask: might allocate an extra sigset_t (128 bytes).
	 - __libc_sigaction: allocate a struct kernel_sigaction (144 bytes on
	   64-bit, 136 on 32-bit).
	 - __sched_setparam, __sched_setscheduler, __setsig, __setpgid,
	   local_seteuid, local_setegid, __close_nocancel, __getrlimit64,
	   __close_nocancel, __open_nocancel, __dup2, __chdir, __fchdir:
	   and direct syscall.
	 - __fcntl: wrapper only uses local variables.
	 - spawn_closefrom: uses up to 1024 bytes as local buffer
	   - __direntries_read
	     - __getdents64: MIPS64 uses up to buffer size used, 1024 in this
	       specific usage.
	   - __direntries_next: local variables.
	   - __close_nocancel: direct syscall.
         - execvpe allocates at least (NAME_MAX + 1) + PATH_MAX to create the
	   combination of PATH entry and program name (1024 + 255 + 1).

	 It allocates 2048 plus some stack for automatic variables and function
	 calls.  */
      char stack[2560];
      ec = spawni_clone (args, stack, sizeof stack, pid);
    }

  return ec;
}

/* Spawn a new process executing PATH with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS. */
int
__spawni (pid_t * pid, const char *file,
	  const posix_spawn_file_actions_t * file_actions,
	  const posix_spawnattr_t * attrp, char *const argv[],
	  char *const envp[], int xflags)
{
  /* Child must set args.err to something non-negative - we rely on
     the parent and child sharing VM.  */
  struct posix_spawn_args args = {
    .err = 0,
    .file = file,
    .fa = file_actions,
    .attr = attrp ? attrp : &(const posix_spawnattr_t) { 0 },
    .argv = argv,
    .envp = envp,
    .xflags = xflags
  };

  /* Disable asynchronous cancellation.  */
  int state;
  __libc_ptf_call (__pthread_setcancelstate,
                   (PTHREAD_CANCEL_DISABLE, &state), 0);

  __libc_signal_block_all (&args.oldmask);

  int ec = spawn_process (&args, pid);

  __libc_signal_restore_set (&args.oldmask);

  __libc_ptf_call (__pthread_setcancelstate, (state, NULL), 0);

  return ec;
}
