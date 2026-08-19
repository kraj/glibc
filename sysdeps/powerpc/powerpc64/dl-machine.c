/* Machine-dependent ELF dynamic relocation functions.  PowerPC64 version.
   Copyright (C) 1995-2024 Free Software Foundation, Inc.
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
#include <dl-machine.h>

#if _CALL_ELF == 2
void
_dl_error_localentry (struct link_map *map, const Elf64_Sym *refsym)
{
  const char *strtab = (const void *) D_PTR (map, l_info[DT_STRTAB]);

  struct dl_exception exc;
  _dl_exception_create_format (&exc, map->l_name, "expected localentry:0 `%s'",
			       strtab + refsym->st_name);
  _dl_signal_exception (0, &exc, NULL);
}
#endif
