/* Copyright (C) 1999-2026 Free Software Foundation, Inc.
   Copyright The GNU Toolchain Authors.
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

#include <elf-read-prop.h>

/* This code is a heavily simplified version of the readelf program
   that's part of the current binutils development version.  For architectures
   which need to handle both 32bit and 64bit ELF libraries,  this file is
   included twice for each arch size.  */

/* check_ptr checks that a pointer is in the mmaped file and doesn't
   point outside it.  */
#undef check_ptr
#define check_ptr(ptr)						\
do								\
  {								\
    if ((void *)(ptr) < file_contents				\
	|| (void *)(ptr) > (file_contents+file_length))		\
      {								\
	error (0, 0, _("file %s is truncated\n"), file_name);	\
	return 1;						\
      }								\
  }								\
 while (0);

/* Returns 0 if everything is ok, != 0 in case of error.  */
int
process_elf_file (const char *file_name, const char *lib, int *flag,
		  unsigned int *isa_level, char **soname, void *file_contents,
		  size_t file_length)
{
  int i;
  unsigned int dynamic_addr;
  size_t dynamic_size;
  char *program_interpreter;

  ElfW(Ehdr) *elf_header;
  ElfW(Phdr) *elf_pheader, *segment;
  ElfW(Dyn) *dynamic_segment, *dyn_entry;
  char *dynamic_strings;

  elf_header = (ElfW(Ehdr) *) file_contents;

  if (elf_header->e_ident [EI_CLASS] != ElfW (CLASS))
    {
      if (opt_verbose)
	{
	  if (elf_header->e_ident [EI_CLASS] == ELFCLASS32)
	    error (0, 0, _("%s is a 32 bit ELF file.\n"), file_name);
	  else if (elf_header->e_ident [EI_CLASS] == ELFCLASS64)
	    error (0, 0, _("%s is a 64 bit ELF file.\n"), file_name);
	  else
	    error (0, 0, _("Unknown ELFCLASS in file %s.\n"), file_name);
	}
      return 1;
    }

  if (elf_header->e_type != ET_DYN)
    {
      error (0, 0, _("%s is not a shared object file (Type: %d).\n"), file_name,
	     elf_header->e_type);
      return 1;
    }

  /* Get information from elf program header.  */
  elf_pheader = (ElfW(Phdr) *) (elf_header->e_phoff + file_contents);
  check_ptr (elf_pheader);

  /* The library is an elf library.  */
  *flag = FLAG_ELF_LIBC6;

  /* The default ISA level is 0.  */
  *isa_level = 0;

  dynamic_addr = 0;
  dynamic_size = 0;
  program_interpreter = NULL;
  for (i = 0, segment = elf_pheader;
       i < elf_header->e_phnum; i++, segment++)
    {
      check_ptr (segment);

      switch (segment->p_type)
	{
	case PT_DYNAMIC:
	  if (dynamic_addr)
	    error (0, 0, _("more than one dynamic segment\n"));

	  dynamic_addr = segment->p_offset;
	  dynamic_size = segment->p_filesz;
	  break;

	case PT_INTERP:
	  program_interpreter = (char *) (file_contents + segment->p_offset);
	  check_ptr (program_interpreter);
	  break;

	case PT_GNU_PROPERTY:
	  /* The NT_GNU_PROPERTY_TYPE_0 note must be aligned to 4 bytes
	     in 32-bit objects and to 8 bytes in 64-bit objects.  Skip
	     notes with incorrect alignment.  */
	  if (segment->p_align == (__ELF_NATIVE_CLASS / 8))
	    {
	      /* The segment must lie completely within the mapped file.  */
	      if (segment->p_offset > file_length
		  || segment->p_filesz > file_length - segment->p_offset)
		break;

	      const unsigned char *seg
		= (const unsigned char *) file_contents + segment->p_offset;
	      const size_t seg_size = segment->p_filesz;
	      const size_t align = segment->p_align;
	      size_t off = 0;	/* Offset of the current note.  */
	      unsigned int last_type = 0;

	      /* Track the note and property positions as size_t offsets
		 within the segment and check every name, descriptor and
		 padding length against the remaining length before
		 advancing.  Do not compare pointers truncated to
		 ElfW(Addr), which is 32 bits in process_elf32_file.  */
	      while (seg_size - off >= sizeof (ElfW(Nhdr)))
		{
		  const ElfW(Nhdr) *note = (const void *) (seg + off);
		  size_t left = seg_size - off;	/* >= sizeof (ElfW(Nhdr)).  */

		  /* The owner name and its padding must fit in the
		     segment.  raw is the unaligned end offset of the
		     name.  */
		  if (note->n_namesz > left - sizeof (ElfW(Nhdr)))
		    break;
		  size_t raw = sizeof (ElfW(Nhdr)) + note->n_namesz;
		  size_t pad = -raw & (align - 1);
		  if (pad > left - raw)
		    break;
		  size_t desc_off = raw + pad;

		  /* The descriptor must fit in the segment.  */
		  if (note->n_descsz > left - desc_off)
		    break;

		  /* Find the NT_GNU_PROPERTY_TYPE_0 note.  */
		  if (note->n_namesz == 4
		      && note->n_type == NT_GNU_PROPERTY_TYPE_0
		      && memcmp (note + 1, "GNU", 4) == 0)
		    {
		      /* Check for invalid property.  */
		      if (note->n_descsz < 8
			  || (note->n_descsz % sizeof (ElfW(Addr))) != 0)
			goto done;

		      /* Start and length of the property array.  */
		      const unsigned char *desc = seg + off + desc_off;
		      size_t dleft = note->n_descsz;
		      size_t poff = 0;

		      do
			{
			  unsigned int type
			    = *(const unsigned int *) (desc + poff);
			  unsigned int datasz
			    = *(const unsigned int *) (desc + poff + 4);

			  /* Property type must be in ascending order.  */
			  if (type < last_type)
			    goto done;

			  poff += 8;
			  if (datasz > dleft - poff)
			    goto done;

			  last_type = type;

			  /* Target specific property processing.
			     Return value:
			       false: Continue processing the properties.
			       true : Stop processing the properties.
			   */
			  if (read_gnu_property (isa_level, type,
						 datasz,
						 (void *) (desc + poff)))
			    goto done;

			  /* Check the next property item.  */
			  poff += ALIGN_UP (datasz, sizeof (ElfW(Addr)));
			}
		      while (dleft - poff >= 8);

		      /* Only handle one NT_GNU_PROPERTY_TYPE_0.  */
		      goto done;
		    }

		  /* Advance to the next note.  raw is the unaligned end
		     offset of the descriptor.  If the padding reaches
		     the end of the segment there is no next note.  */
		  raw = desc_off + note->n_descsz;
		  pad = -raw & (align - 1);
		  if (pad >= left - raw)
		    break;
		  off += raw + pad;
		}
	    }
done:
	  break;

	default:
	  break;
	}

    }

  /* Now we can read the dynamic sections.  */
  if (dynamic_size == 0)
    return 1;

  dynamic_segment = (ElfW(Dyn) *) (file_contents + dynamic_addr);
  check_ptr (dynamic_segment);

  /* Find the string table.  */
  dynamic_strings = NULL;
  for (dyn_entry = dynamic_segment; dyn_entry->d_tag != DT_NULL;
       ++dyn_entry)
    {
      check_ptr (dyn_entry);
      if (dyn_entry->d_tag == DT_STRTAB)
	{
	  /* Find the file offset of the segment containing the dynamic
	     string table.  */
	  ElfW(Off) loadoff = -1;
	  for (i = 0, segment = elf_pheader;
	       i < elf_header->e_phnum; i++, segment++)
	    {
	      if (segment->p_type == PT_LOAD
		  && dyn_entry->d_un.d_val >= segment->p_vaddr
		  && (dyn_entry->d_un.d_val - segment->p_vaddr
		      < segment->p_filesz))
		{
		  loadoff = segment->p_vaddr - segment->p_offset;
		  break;
		}
	    }
	  if (loadoff == (ElfW(Off)) -1)
	    {
	      /* Very strange. */
	      loadoff = 0;
	    }

	  dynamic_strings = (char *) (file_contents + dyn_entry->d_un.d_val
				      - loadoff);
	  check_ptr (dynamic_strings);
	  break;
	}
    }

  if (dynamic_strings == NULL)
    return 1;

  /* Now read the DT_SONAME entries.  */
  for (dyn_entry = dynamic_segment; dyn_entry->d_tag != DT_NULL;
       ++dyn_entry)
    {
      if (dyn_entry->d_tag == DT_SONAME)
	{
	  char *name = dynamic_strings + dyn_entry->d_un.d_val;
	  check_ptr (name);
          *soname = xstrdup (name);
          return 0;
	}
    }

  return 0;
}
