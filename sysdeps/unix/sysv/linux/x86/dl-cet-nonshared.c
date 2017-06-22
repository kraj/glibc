/* Linux/x86 CET setup function for static executable.
   This file is part of the GNU C Library.
   Copyright (C) 2017 Free Software Foundation, Inc.

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


#ifndef SHARED
# include <link.h>
# include <ldsodefs.h>
# include "dl-cet.h"

/* Set up IBT and SHSTK for static executable.   */

void
_dl_setup_cet (const ElfW(Phdr) *phdr, size_t phnum,
	       const ElfW(Addr) addr)
{
  const struct cpu_features *cpu_features = __get_cpu_features ();
  size_t i;

  for (i = 0; i < phnum; i++)
    if (phdr[i].p_type == PT_NOTE)
      {
	const ElfW(Addr) start = phdr[i].p_vaddr + addr;
	const ElfW(Nhdr) *note = (const void *) start;

	while ((ElfW(Addr)) (note + 1) - start < phdr[i].p_memsz)
	  {
	    /* Find the NT_GNU_PROPERTY_TYPE_0 note.  */
	    if (note->n_namesz == 4
		&& note->n_type == NT_GNU_PROPERTY_TYPE_0
		&& memcmp (note + 1, "GNU", 4) == 0)
	      {
		/* Check for invalid property.  */
		if (note->n_descsz < 8
		    || (note->n_descsz % sizeof (ElfW(Addr))) != 0)
		  break;

		/* Start and end of property array.  */
		unsigned char *ptr = (unsigned char *) (note + 1) + 4;
		unsigned char *ptr_end = ptr + note->n_descsz;

		while (1)
		  {
		    unsigned int type = *(unsigned int *) ptr;
		    unsigned int datasz = *(unsigned int *) (ptr + 4);

		    ptr += 8;
		    if ((ptr + datasz) > ptr_end)
		      break;

		    if (type == GNU_PROPERTY_X86_FEATURE_1_AND
			&& datasz == 4)
		      {
			unsigned int feature_1 = ptr[0];

			if ((feature_1 & GNU_PROPERTY_X86_FEATURE_1_IBT))
			  {
			    if (CPU_FEATURES_CPU_P (cpu_features, IBT))
			      {
				/* FIXME: Set up IBT.  */
			      }
			    else
			      /* Disable IBT.  */
			      feature_1 &= ~GNU_PROPERTY_X86_FEATURE_1_IBT;
			  }

			if ((feature_1 & GNU_PROPERTY_X86_FEATURE_1_SHSTK))
			  {
			    if (CPU_FEATURES_CPU_P (cpu_features, SHSTK))
			      {
				/* FIXME: Set up SHSTK.  */
			      }
			    else
			      /* Disable SHSTK.  */
			      feature_1 &= ~GNU_PROPERTY_X86_FEATURE_1_SHSTK;
			  }

			GL(dl_x86_feature_1) = feature_1;
			return;
		      }
		  }
	      }
	    note = ((const void *) (note + 1)
		    + ROUND_NOTE (note->n_namesz)
		    + ROUND_NOTE (note->n_descsz));
	  }
      }
}
#endif
