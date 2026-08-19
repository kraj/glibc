/* Relocation overflow reporting for POWER.
   Copyright (C) 2026 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

#include <ldsodefs.h>

void
_dl_reloc_overflow (struct link_map *map, const char *name,
		    ElfW(Addr) *const reloc_addr, const ElfW(Sym) *refsym)
{
  struct dl_exception exc;

  if (refsym != NULL)
    {
      const char *strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);
      _dl_exception_create_format
	(&exc, map->l_name, "%s reloc at 0x%lx for symbol `%s' out of range",
	 name, (unsigned long int) reloc_addr, strtab + refsym->st_name);
    }
  else
    _dl_exception_create_format
      (&exc, map->l_name, "%s reloc at 0x%lx out of range",
       name, (unsigned long int) reloc_addr);

  _dl_signal_exception (0, &exc, NULL);
}
