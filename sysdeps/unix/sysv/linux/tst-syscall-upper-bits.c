/* Check that system call wrappers extend 32-bit arguments (BZ 34917).
   Copyright (C) 2026 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <support/check.h>
#include <support/xunistd.h>

/* Some ABI (x86_64, aarch64) leave the upper 32 bits of a register holding
   a 32-bit argument unspecified.  Linux declares some arguments that glibc
   takes as int or unsigned int as long or unsigned long, and rejects stray
   upper bits.  So the wrapper must extend them.  */

/* Return V with garbage in the upper 32 bits.  The asm hides the value from
   the compiler, which could otherwise drop the upper bits because the
   call_* functions only use the lower 32 bits.  */
#define DIRTY(v)							\
  ({									\
    uint64_t __dirty = ((uint64_t) (unsigned int) (v)			\
			| UINT64_C (0xdeadbeef00000000));		\
    asm ("" : "+r" (__dirty));						\
    __dirty;								\
  })

/* Check that DIRTY, a call with garbage in the upper bits of its 32-bit
   arguments, returns the same value and errno as CLEAN.  This is for
   calls whose outcome depends on the system, such as kernel support or
   a seccomp filter.  */
#define COMPARE_CALLS(clean, dirty)					\
  do									\
    {									\
      errno = 0;							\
      long int ret = (clean);						\
      int err = errno;							\
      errno = 0;							\
      long int dret = (dirty);						\
      int derr = errno;							\
      if (dret != ret || derr != err)					\
	FAIL ("%s: returned %ld (errno %d), expected %ld (errno %d)",	\
	      #dirty, dret, derr, ret, err);				\
    }									\
  while (0)

static int __attribute_noinline__
call_pkey_mprotect (void *addr, size_t len, uint64_t prot, uint64_t pkey)
{
  return pkey_mprotect (addr, len, prot, pkey);
}

/* The kernel rejects unknown prot bits.  */
static void
test_pkey_mprotect (void)
{
  int key = pkey_alloc (0, 0);
  if (key < 0)
    {
      printf ("info: pkey_alloc failed: %m\n");
      return;
    }
  size_t pagesize = getpagesize ();
  void *p = xmmap (NULL, pagesize, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS, -1);
  TEST_COMPARE (call_pkey_mprotect (p, pagesize, DIRTY (PROT_READ),
				    DIRTY (key)), 0);
  xmunmap (p, pagesize);
  pkey_free (key);
}

static int __attribute_noinline__
call_mprotect (void *addr, size_t len, uint64_t prot)
{
  return mprotect (addr, len, prot);
}

/* The kernel rejects unknown prot bits.  */
static void
test_mprotect (void)
{
  size_t pagesize = getpagesize ();
  void *p = xmmap (NULL, pagesize, PROT_READ | PROT_WRITE,
		   MAP_PRIVATE | MAP_ANONYMOUS, -1);
  TEST_COMPARE (call_mprotect (p, pagesize, DIRTY (PROT_READ)), 0);
  TEST_COMPARE (call_mprotect (p, pagesize, DIRTY (PROT_READ | PROT_WRITE)),
		0);
  xmunmap (p, pagesize);
}

static int __attribute_noinline__
call_pkey_alloc (uint64_t flags, uint64_t access_restrictions)
{
  return pkey_alloc (flags, access_restrictions);
}

/* The kernel rejects nonzero flags even if protection keys are not
   supported.  */
static void
test_pkey_alloc (void)
{
  errno = 0;
  int key = pkey_alloc (0, 0);
  int err = errno;
  errno = 0;
  int dkey = call_pkey_alloc (DIRTY (0), DIRTY (0));
  int derr = errno;
  if (key < 0)
    {
      TEST_COMPARE (dkey, -1);
      TEST_COMPARE (derr, err);
    }
  else
    {
      TEST_VERIFY (dkey >= 0);
      pkey_free (key);
      if (dkey >= 0)
	pkey_free (dkey);
    }
}

static long int __attribute_noinline__
call_ptrace (uint64_t request, pid_t pid, void *addr, void *data)
{
  return ptrace ((enum __ptrace_request) request, pid, addr, data);
}

static int
traceme (bool dirty)
{
  pid_t pid = xfork ();
  if (pid == 0)
    {
      long int ret;
      if (dirty)
	ret = call_ptrace (DIRTY (PTRACE_TRACEME), 0, NULL, NULL);
      else
	ret = ptrace (PTRACE_TRACEME, 0, NULL, NULL);
      _exit (ret == 0 ? 0 : errno);
    }
  int status;
  xwaitpid (pid, &status, 0);
  TEST_VERIFY (WIFEXITED (status));
  return WEXITSTATUS (status);
}

/* The kernel compares the request as a long.  */
static void
test_ptrace (void)
{
  COMPARE_CALLS (traceme (false), traceme (true));
}

static int
do_test (void)
{
  test_mprotect ();
  test_pkey_mprotect ();
  test_pkey_alloc ();
  test_ptrace ();

  return 0;
}

#include <support/test-driver.c>
