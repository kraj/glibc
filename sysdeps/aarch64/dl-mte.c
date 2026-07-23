/* AArch64 implementation for MTE (memory tagging).
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

#include <sys/cdefs.h>
#include <ldsodefs.h>

/* For the MTE prctl syscall.  */
#define PR_SET_TAGGED_ADDR_CTRL 55
#define PR_MTE_TAG_SHIFT        3
#define PR_TAGGED_ADDR_ENABLE   (1UL << 0)
#define PR_MTE_TCF_SYNC         (1UL << 1)
#define PR_MTE_TCF_ASYNC        (1UL << 2)

/* The maximal set of permitted tags that the MTE random tag generation
   instruction may use.  We exclude tag 0 because a) we want to reserve
   that for the libc heap structures and b) because it makes it easier
   to see when pointer have been correctly tagged.  */
#define MTE_ALLOWED_TAGS        (0xfffe << PR_MTE_TAG_SHIFT)

void __mte_init (void);
rtld_hidden_proto (__mte_init)

void __mte_init (void)
{
  unsigned int mode = GL (dl_aarch64_mte_mode);
  if (mode == MTE_MODE_DISABLED)
    return;
  uint64_t flags = PR_TAGGED_ADDR_ENABLE | MTE_ALLOWED_TAGS;
  switch (mode)
    {
    case MTE_MODE_ENABLED:
      flags |= PR_MTE_TCF_SYNC | PR_MTE_TCF_ASYNC;
      break;
    case MTE_MODE_SYNC:
      flags |= PR_MTE_TCF_SYNC;
      break;
    case MTE_MODE_ASYNC:
      flags |= PR_MTE_TCF_ASYNC;
      break;
    default:
      _dl_fatal_printf ("unknown MTE mode: %d\n", mode);
    }
  /* We use inline system call to avoid unnecessary dependency
     on the sys/prctl.h header.  */
  int r = INLINE_SYSCALL_CALL (prctl, PR_SET_TAGGED_ADDR_CTRL, flags, 0, 0, 0);
  if (r == -1)
    _dl_fatal_printf ("failed to enable MTE\n");
}
rtld_hidden_def (__mte_init)

void
_dl_mte_check (struct link_map *l, const char *program)
{
  /* MTE binary marking can be checked here when support for memtag ABI
     is added.  When the main binary has no MTE marking, we enable only
     standard protection when requested MTE mode is not 'none'.
     Currently standard protection includes: heap.  */
  if (GL (dl_aarch64_mte_mode) == MTE_MODE_DISABLED)
    GL (dl_aarch64_mte_scope) = MTE_SCOPE_NONE;
  else
    GL (dl_aarch64_mte_scope) = MTE_SCOPE_STANDARD;
}
