/* AArch64 MTE support.
   Copyright (C) 2026 Free Software Foundation, Inc.

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

#include <assert.h>
#include <ldsodefs.h>
#include <sys/auxv.h>
#include <dl-tunables.h>
#include <dl-mte.h>
#include <dl-prop.h>

#pragma GCC optimize ("O0")

/* The maximal set of permitted tags that the MTE random tag generation
   instruction may use.  We exclude tag 0 because a) we want to reserve
   that for the libc heap structures and b) because it makes it easier
   to see when pointer have been correctly tagged.  */
#define MTE_ALLOWED_TAGS (0xfffe << PR_MTE_TAG_SHIFT)

#ifdef USE_AARCH64_MEMTAG_ABI

# define DT_AARCH64(x) (DT_AARCH64_##x - DT_LOPROC + DT_NUM)

static void
fail (const struct link_map *l, const char *program, const char *mode)
{
  if (program != NULL)
    {
      if (program[0] != '\0' && l->l_name[0] != '\0')
	_dl_fatal_printf ("%s: %s: MTE protection %s is not supported\n",
			  program, l->l_name, mode);
      if (program[0] != '\0')
        _dl_fatal_printf ("%s: MTE protection %s is not supported\n",
			  program, mode);
      _dl_fatal_printf ("error: MTE protection %s is not supported\n",
			mode);
  }
  else
    _dl_signal_error (0, l->l_name, "dlopen", "MTE is not enabled");
}

static void
fail_mode (const struct link_map *l, const char *program, uint64_t mode)
{
  assert (program != NULL);
  if (program[0] != '\0' && l->l_name[0] != '\0')
    _dl_fatal_printf ("%s: %s: MTE mode 0x%lx not supported\n",
		      program, l->l_name, mode);
  if (program[0] != '\0')
    _dl_fatal_printf ("%s: MTE mode 0x%lx not supported\n", program, mode);
  _dl_fatal_printf ("error: MTE mode 0x%lx not supported\n", mode);
}

static void
unsupported_stack (void)
{
  _dl_fatal_printf ("error: MTE stack required, but kernel does not support MTE\n");
}

static bool requires_mte_heap (const struct link_map *l)
{
  return l->l_info[DT_AARCH64 (MEMTAG_HEAP)] != NULL;
}

static bool requires_mte_stack (const struct link_map *l)
{
  return l->l_info[DT_AARCH64 (MEMTAG_STACK)] != NULL;
}

static bool requires_mte_globals (const struct link_map *l)
{
  return l->l_info[DT_AARCH64 (MEMTAG_GLOBALS)] != NULL
    || l->l_info[DT_AARCH64 (MEMTAG_GLOBALSSZ)] != NULL;
}

static void check_mte_mode (const struct link_map *l, const char *program)
{
  /* Only the executable are considered for the MTE mode.  */
  const ElfW(Dyn) *d = l->l_info[DT_AARCH64 (MEMTAG_MODE)];
  if (d != NULL && l->l_type == lt_executable)
    {
      if (d->d_un.d_val == 0)
	GL(dl_aarch64_mte) |= MTE_MODE_SYNC;
      else if (d->d_un.d_val == 1)
	GL(dl_aarch64_mte) |= MTE_MODE_ASYNC;
      else
	fail_mode (l, program, d->d_un.d_val);
    }
  else
    /* Use sync by default or if the dynamic tag is not present.  */
    GL(dl_aarch64_mte) |= MTE_MODE_ASYNC;
}

static void check_mte (const struct link_map *l, const char *program)
{
  if (requires_mte_stack (l))
    GL(dl_aarch64_mte) |= MTE_STACK;
  if (requires_mte_heap (l))
    fail (l, program, "heap");
  if (requires_mte_globals (l))
    fail (l, program, "globals");
}
#endif

static inline uint8_t mte_mode (void)
{
  return GL(dl_aarch64_mte) & MTE_MODE_MASK;
}

static inline bool enable_mte_stack (void)
{
#ifdef USE_AARCH64_MEMTAG_ABI
  return GL(dl_aarch64_mte) & MTE_STACK;
#else
  return false;
#endif
}

static inline bool enable_mte_heap (void)
{
#ifdef USE_MTAB
  return GL(dl_aarch64_mte) & MTE_HEAD;
#else
  return false;
#endif
}

static inline bool enable_mte (void)
{
  return enable_mte_stack () || enable_mte_heap ();
}

void
_dl_mte_check (struct link_map *l, const char *program)
{
#ifdef USE_AARCH64_MEMTAG_ABI
  check_mte_mode (l, program);

  check_mte (l, program);
  for (unsigned int i = 0; i < l->l_searchlist.r_nlist; i++)
    check_mte (l->l_searchlist.r_list[i], program);

  /* For dlopen, if program has not enabled MTE stack at the startup, signal
     that the module can not be loaded.  */
  if (enable_mte_stack () && program == NULL)
    {
      GL(dl_aarch64_mte) &= ~MTE_STACK;

      fail (l, program, "stack");
    }
#endif
}

void
_dl_mte_init (void)
{
#if defined USE_MTAG || defined USE_AARCH64_MEMTAG_ABI
  if ((GLRO (dl_hwcap2) & HWCAP2_MTE) == 0 || !enable_mte ())
    {
# ifdef USE_AARCH64_MEMTAG_ABI
      if (GL(dl_aarch64_mte) & MTE_STACK)
	unsupported_stack ();
# endif
      GL(dl_aarch64_mte) = 0;
      return;
    }

#ifdef USE_MTAG
  /* The tunable can override the MTE mode from the MEMTAG ABI.  */
  if (TUNABLE_IS_INITIALIZED_FULL (glibc, mem, tagging))
    {
      int32_t mte_tunable = TUNABLE_GET_FULL (glibc, mem, tagging, int32_t,
					      NULL);
      if (mte_tunable & 0x1)
	GL(dl_aarch64_mte) |= MTE_HEAP;
      if (mte_tunable & 0x2)
	{
	  GL(dl_aarch64_mte) &= ~MTE_MODE_ASYNC;
	  GL(dl_aarch64_mte) |= MTE_MODE_SYNC;
	}
      if (mte_tunable & 0x4)
	{
	  /* The documentatation states that it is either precise or deferred
	     faulting mode, so let the kernel handle it.  */
	  GL(dl_aarch64_mte) &= ~(MTE_MODE_ASYNC | MTE_MODE_SYNC);
	  GL(dl_aarch64_mte) |= MTE_MODE_AUTO;
	}
    }
#endif

  uint64_t flags = PR_TAGGED_ADDR_ENABLE | MTE_ALLOWED_TAGS;
  switch (mte_mode ())
    {
    case MTE_MODE_SYNC:
      flags |= PR_MTE_TCF_SYNC;
      break;
    case MTE_MODE_AUTO:
      flags |= PR_MTE_TCF_SYNC | PR_MTE_TCF_ASYNC;
      break;
    case MTE_MODE_ASYNC:
      flags |= PR_MTE_TCF_ASYNC;
      break;
    }

  int r = INLINE_SYSCALL_CALL (prctl, PR_SET_TAGGED_ADDR_CTRL, flags, 0, 0, 0);
  if (r == -1)
     _dl_fatal_printf ("failed to enable MTE: %d\n", -r);
#endif

#ifdef USE_AARCH64_MEMTAG_ABI
  if (enable_mte_stack() && !_dl_mte_setup_stack ())
    _dl_fatal_printf ("error: MTE stack setup failed\n");
#endif
}
