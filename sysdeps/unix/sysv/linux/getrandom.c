/* Implementation of the getrandom system call.
   Copyright (C) 2016-2024 Free Software Foundation, Inc.
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
   <https://www.gnu.org/licenses/>.  */

#include <sys/random.h>
#include <errno.h>
#include <unistd.h>
#include <sysdep-cancel.h>

static inline ssize_t
getrandom_syscall (void *buffer, size_t length, unsigned int flags,
		   bool cancel)
{
  return cancel
	 ? SYSCALL_CANCEL (getrandom, buffer, length, flags)
	 : INLINE_SYSCALL_CALL (getrandom, buffer, length, flags);
}

#ifdef HAVE_GETRANDOM_VSYSCALL
# include <getrandom_vdso.h>
# include <ldsodefs.h>
# include <libc-lock.h>
# include <list.h>
# include <setvmaname.h>
# include <sys/mman.h>
# include <sys/sysinfo.h>
# include <tls-internal.h>

# define ALIGN_PAGE(p)		PTR_ALIGN_UP (p, GLRO (dl_pagesize))
# define READ_ONCE(p)		(*((volatile typeof (p) *) (&(p))))
# define WRITE_ONCE(p, v)	(*((volatile typeof (p) *) (&(p))) = (v))
# define RESERVE_PTR(p)		((void *) ((uintptr_t) (p) | 1UL))
# define RELEASE_PTR(p)		((void *) ((uintptr_t) (p) & ~1UL))
# define IS_RESERVED_PTR(p)	(!!((uintptr_t) (p) & 1UL))

static struct
{
  __libc_lock_define (, lock);

  void **states;  /* Queue of opaque states allocated with the kernel
		     provided flags and used on getrandom vDSO call.  */
  size_t len;	  /* Number of available free states in the queue.  */
  size_t total;	  /* Number of states allocated from the kernel.  */
  size_t cap;     /* Total numver of states that 'states' can hold before
		     needed to be resized.  */
} grnd_alloc = {
  .lock = LLL_LOCK_INITIALIZER
};

static bool
vgetrandom_get_state_alloc (void)
{
  size_t num = __get_nprocs (); /* Just a decent heuristic.  */

  size_t block_size = ALIGN_PAGE (num * GLRO(dl_vdso_getrandom_state_size));
  num = (GLRO (dl_pagesize) / GLRO(dl_vdso_getrandom_state_size)) *
	(block_size / GLRO (dl_pagesize));
  void *block = __mmap (NULL, block_size, GLRO(dl_vdso_getrandom_mmap_prot),
			GLRO(dl_vdso_getrandom_mmap_flags), -1, 0);
  if (block == MAP_FAILED)
    return false;
  __set_vma_name (block, block_size, " glibc: getrandom");

  if (grnd_alloc.total + num > grnd_alloc.cap)
    {
      /* Use a new mmap instead of trying to mremap.  It avoids a
	 potential multithread fork issue where fork is called just after
	 mremap returns but before assigning to the grnd_alloc.states,
	 thus making the its value invalid in the child.  */
      void *old_states = grnd_alloc.states;
      size_t old_states_size = ALIGN_PAGE (sizeof (*grnd_alloc.states) *
					   grnd_alloc.total + num);
      size_t states_size;
      if (grnd_alloc.states == NULL)
	states_size = old_states_size;
      else
	states_size = ALIGN_PAGE (sizeof (*grnd_alloc.states)
				  * grnd_alloc.cap);

      void **states = __mmap (NULL, states_size, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
      if (states == MAP_FAILED)
	{
	  __munmap (block, block_size);
	  return false;
	}

      /* Atomically replace the old state, so if a fork happens the child
	 process will see a consistent free state buffer.  The size might
	 not be updated, but it does not really matter since the buffer is
	 always increased.  */
      atomic_store_relaxed (&grnd_alloc.states, states);
      if (old_states != NULL)
	__munmap (old_states, old_states_size);

      __set_vma_name (states, states_size, " glibc: getrandom states");
      grnd_alloc.cap = states_size / sizeof (*grnd_alloc.states);
    }

  for (size_t i = 0; i < num; ++i)
    {
      /* States should not straddle a page.  */
      if (((uintptr_t) block & (GLRO (dl_pagesize) - 1)) +
	  GLRO(dl_vdso_getrandom_state_size) > GLRO (dl_pagesize))
	block = ALIGN_PAGE (block);
      grnd_alloc.states[i] = block;
      block += GLRO(dl_vdso_getrandom_state_size);
    }
  grnd_alloc.len = num;
  grnd_alloc.total += num;

  return true;
}

/* Allocate an opaque state for vgetrandom.  If the grnd_alloc does not have
   any, mmap() another page of them using the vgetrandom parameters.  */
static void *
vgetrandom_get_state (void)
{
  void *state = NULL;

  /* The signal blocking avoid the potential issue where _Fork() (which is
     async-signal-safe) is called with the lock taken.  The function is
     called only once during thread lifetime, so the overhead should be
     minimal.  */
  internal_sigset_t set;
  internal_signal_block_all (&set);
  __libc_lock_lock (grnd_alloc.lock);

  if (grnd_alloc.len > 0 || vgetrandom_get_state_alloc ())
    state = grnd_alloc.states[--grnd_alloc.len];

  __libc_lock_unlock (grnd_alloc.lock);
  internal_signal_restore_set (&set);

  return state;
}

/* Returns true when vgetrandom is used successfully.  Returns false if the
   syscall fallback should be issued in the case the vDSO is not present, in
   the case of reentrancy, or if any memory allocation fails.  */
static ssize_t
getrandom_vdso (void *buffer, size_t length, unsigned int flags, bool cancel)
{
  if (GLRO (dl_vdso_getrandom_state_size) == 0)
    return getrandom_syscall (buffer, length, flags, cancel);

  struct pthread *self = THREAD_SELF;

  /* If the LSB of getrandom_buf is set, then this function is already being
     called, and we have a reentrant call from a signal handler.  In this case
     fallback to the syscall.  */
  void *state = READ_ONCE (self->getrandom_buf);
  if (IS_RESERVED_PTR (state))
    return getrandom_syscall (buffer, length, flags, cancel);
  WRITE_ONCE (self->getrandom_buf, RESERVE_PTR (state));

  bool r = false;
  if (state == NULL)
    {
      state = vgetrandom_get_state ();
      if (state == NULL)
        goto out;
    }

  /* Since the vDSO fallback does not issue the syscall with the cancellation
     bridge (__syscall_cancel_arch), use GRND_NONBLOCK so there is no
     potential unbounded blocking in the kernel.  It should be a rare
     situation, only at system startup when RNG is not initialized.  */
  ssize_t ret =  GLRO (dl_vdso_getrandom) (buffer,
					   length,
					   flags | GRND_NONBLOCK,
					   state,
					   GLRO(dl_vdso_getrandom_state_size));
  if (INTERNAL_SYSCALL_ERROR_P (ret))
    {
      /* Fallback to the syscall if the kernel would block.  */
      int err = INTERNAL_SYSCALL_ERRNO (ret);
      if (err == EAGAIN && !(flags & GRND_NONBLOCK))
        goto out;

      __set_errno (err);
      ret = -1;
    }
  r = true;

out:
  WRITE_ONCE (self->getrandom_buf, state);
  return r ? ret : getrandom_syscall (buffer, length, flags, cancel);
}
#endif

/* Re-add the state state from CURP on the free list.  */
void
__getrandom_reset_state (struct pthread *curp)
{
#ifdef HAVE_GETRANDOM_VSYSCALL
  if (grnd_alloc.states == NULL || curp->getrandom_buf == NULL)
    return;
  grnd_alloc.states[grnd_alloc.len++] = RELEASE_PTR (curp->getrandom_buf);
  curp->getrandom_buf = NULL;
#endif
}

/* Called when a thread terminates, and adds its random buffer back into the
   allocator pool for use in a future thread.  */
void
__getrandom_vdso_release (struct pthread *curp)
{
#ifdef HAVE_GETRANDOM_VSYSCALL
  if (curp->getrandom_buf == NULL)
    return;

  __libc_lock_lock (grnd_alloc.lock);
  grnd_alloc.states[grnd_alloc.len++] = curp->getrandom_buf;
  __libc_lock_unlock (grnd_alloc.lock);
#endif
}

/* Reset the internal lock state in case another thread has locked while
   this thread calls fork.  The stale thread states will be handled by
   reclaim_stacks which calls __getrandom_reset_state on each thread.  */
void
__getrandom_fork_subprocess (void)
{
#ifdef HAVE_GETRANDOM_VSYSCALL
  grnd_alloc.lock = LLL_LOCK_INITIALIZER;
#endif
}

ssize_t
__getrandom_nocancel (void *buffer, size_t length, unsigned int flags)
{
#ifdef HAVE_GETRANDOM_VSYSCALL
  return getrandom_vdso (buffer, length, flags, false);
#else
  return getrandom_syscall (buffer, length, flags, false);
#endif
}

/* Write up to LENGTH bytes of randomness starting at BUFFER.
   Return the number of bytes written, or -1 on error.  */
ssize_t
__getrandom (void *buffer, size_t length, unsigned int flags)
{
#ifdef HAVE_GETRANDOM_VSYSCALL
  return getrandom_vdso (buffer, length, flags, true);
#else
  return getrandom_syscall (buffer, length, flags, true);
#endif
}
libc_hidden_def (__getrandom)
weak_alias (__getrandom, getrandom)
