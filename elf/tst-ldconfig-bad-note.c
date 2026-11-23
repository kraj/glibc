/* Smoke-test ldconfig with malformed PT_GNU_PROPERTY notes (bug 34954).
   Copyright The GNU Toolchain Authors.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <https://www.gnu.org/licenses/>.  */

/* Each test case below places a crafted ELF object in its own directory
   and runs "ldconfig -i -n <dir>" on it in a separate subprocess,
   asserting that ldconfig exits normally.  The malformed objects only
   contain deliberately invalid program-header or note-header fields;
   they never allocate the sizes they claim.

   The objects are built with the native Elf32 and Elf64 structures (so
   they use the byte order of the target), and e_ident, e_machine and
   e_flags are copied from the test executable itself through
   __ehdr_start, so each target recognizes its own ABI.  Most cases are
   built in both ELF classes; on hosts whose dispatcher rejects the
   other class or machine, ldconfig just prints a diagnostic and skips
   the file, which still exits 0.  */

#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/support.h>
#include <support/xunistd.h>

extern const ElfW(Ehdr) __ehdr_start
  __attribute__ ((visibility ("hidden")));

/* A fixed-size buffer holding an ELF image.  */
struct buf
{
  unsigned char b[8192];
  size_t n;
};

static void
bput (struct buf *t, const void *d, size_t n)
{
  TEST_VERIFY_EXIT (n <= sizeof (t->b) - t->n);
  memcpy (t->b + t->n, d, n);
  t->n += n;
}

/* A GNU property header; the layout is the same in both ELF classes.  */
struct gnu_property
{
  uint32_t type;
  uint32_t datasz;
};

/* Append a zero-filled note header; the layout is the same in both ELF
   classes.  */
static void
put_nhdr (struct buf *t, uint32_t namesz, uint32_t descsz, uint32_t type)
{
  Elf32_Nhdr n;
  memset (&n, 0, sizeof n);
  n.n_namesz = namesz;
  n.n_descsz = descsz;
  n.n_type = type;
  bput (t, &n, sizeof n);
}

/* Append the padding that follows a note name or descriptor: raw is the
   unaligned end offset of the name (or descriptor) relative to the note
   header start, and align is the note alignment (4 or 8).  */
static void
put_pad (struct buf *t, size_t raw, size_t align)
{
  static const unsigned char zeros[16];
  size_t pad = -raw & (align - 1);
  bput (t, zeros, pad);
}

static void
put_ehdr (struct buf *t, int elfclass, int phnum)
{
  if (elfclass == ELFCLASS64)
    {
      Elf64_Ehdr e;
      memset (&e, 0, sizeof e);
      memcpy (&e.e_ident, __ehdr_start.e_ident, EI_NIDENT);
      e.e_ident[EI_CLASS] = ELFCLASS64;
      e.e_type = ET_DYN;
      e.e_machine = __ehdr_start.e_machine;
      e.e_version = EV_CURRENT;
      e.e_entry = 0;
      e.e_phoff = sizeof e;
      e.e_ehsize = sizeof e;
      e.e_phentsize = sizeof (Elf64_Phdr);
      e.e_phnum = phnum;
      e.e_flags = __ehdr_start.e_flags;
      bput (t, &e, sizeof e);
    }
  else
    {
      Elf32_Ehdr e;
      memset (&e, 0, sizeof e);
      memcpy (&e.e_ident, __ehdr_start.e_ident, EI_NIDENT);
      e.e_ident[EI_CLASS] = ELFCLASS32;
      e.e_type = ET_DYN;
      e.e_machine = __ehdr_start.e_machine;
      e.e_version = EV_CURRENT;
      e.e_entry = 0;
      e.e_phoff = sizeof e;
      e.e_ehsize = sizeof e;
      e.e_phentsize = sizeof (Elf32_Phdr);
      e.e_phnum = phnum;
      e.e_flags = __ehdr_start.e_flags;
      bput (t, &e, sizeof e);
    }
}

static void
put_phdr (struct buf *t, int elfclass, uint32_t type, uint64_t off,
	  uint64_t filesz, uint64_t align)
{
  if (elfclass == ELFCLASS64)
    {
      Elf64_Phdr p;
      memset (&p, 0, sizeof p);
      p.p_type = type;
      p.p_flags = PF_R;
      p.p_offset = off;
      p.p_vaddr = off;
      p.p_paddr = off;
      p.p_filesz = filesz;
      p.p_memsz = filesz;
      p.p_align = align;
      bput (t, &p, sizeof p);
    }
  else
    {
      Elf32_Phdr p;
      memset (&p, 0, sizeof p);
      p.p_type = type;
      p.p_offset = off;
      p.p_vaddr = off;
      p.p_paddr = off;
      p.p_filesz = filesz;
      p.p_memsz = filesz;
      p.p_align = align;
      bput (t, &p, sizeof p);
    }
}

static void
put_dyn (struct buf *t, int elfclass, int64_t tag, uint64_t val)
{
  if (elfclass == ELFCLASS64)
    {
      Elf64_Dyn d;
      memset (&d, 0, sizeof d);
      d.d_tag = tag;
      d.d_un.d_val = val;
      bput (t, &d, sizeof d);
    }
  else
    {
      Elf32_Dyn d;
      memset (&d, 0, sizeof d);
      d.d_tag = tag;
      d.d_un.d_val = val;
      bput (t, &d, sizeof d);
    }
}

static size_t
ehdr_size (int elfclass)
{
  return elfclass == ELFCLASS64 ? sizeof (Elf64_Ehdr) : sizeof (Elf32_Ehdr);
}

static size_t
phdr_size (int elfclass)
{
  return elfclass == ELFCLASS64 ? sizeof (Elf64_Phdr) : sizeof (Elf32_Phdr);
}

static size_t
dyn_size (int elfclass)
{
  return elfclass == ELFCLASS64 ? sizeof (Elf64_Dyn) : sizeof (Elf32_Dyn);
}

static size_t
addr_size (int elfclass)
{
  return elfclass == ELFCLASS64 ? 8 : 4;
}

static size_t
note_align (int elfclass)
{
  return elfclass == ELFCLASS64 ? 8 : 4;
}

static void
execv_wrapper (void *args)
{
  char **argv = args;

  execv (argv[0], argv);
  FAIL_EXIT1 ("execv: %m");
}

/* Write a test case object into its own directory and run
   "ldconfig -i -n" on that directory in a subprocess.  */
static void
run_case (const char *name, const struct buf *t)
{
  char *dir = xasprintf ("/tst-ldconfig-bad-note-%s", name);
  xmkdirp (dir, 0777);
  char *path = xasprintf ("%s/libbadnote-%s.so", dir, name);
  int fd = xopen (path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  xwrite (fd, t->b, t->n);
  xclose (fd);

  char *prog = xasprintf ("%s/ldconfig", support_install_rootsbindir);
  char *args[] = { prog, (char *) "-i", (char *) "-n", dir, NULL };
  struct support_capture_subprocess result
    = support_capture_subprocess (execv_wrapper, args);
  support_capture_subprocess_check (&result, name, 0, sc_allow_stderr);
  support_capture_subprocess_free (&result);
  free (prog);
  free (path);
  free (dir);
}

/* PT_GNU_PROPERTY p_offset points far past the end of the file: the
   offset from the original bug report for 64-bit objects, and half the
   32-bit address space for 32-bit ones.  */
static void
case_r0_offset (int elfclass)
{
  struct buf t = { 0 };
  uint64_t off = elfclass == ELFCLASS64 ? 0x100000000000 : 0x80000000;
  put_ehdr (&t, elfclass, 1);
  put_phdr (&t, elfclass, PT_GNU_PROPERTY, off, 0x100,
	    note_align (elfclass));
  char *name = xasprintf ("r0-offset%d", elfclass == ELFCLASS64 ? 64 : 32);
  run_case (name, &t);
  free (name);
}

/* A complete GNU note header and name claim a descriptor of 0xfffffff8
   bytes, followed by zero fill up to the end of the file; the zero
   properties keep the property loop scanning on the unpatched
   ldconfig.  */
static void
case_r1_descsz (int elfclass)
{
  struct buf t = { 0 };
  size_t seg_off = ehdr_size (elfclass) + phdr_size (elfclass);
  put_ehdr (&t, elfclass, 1);
  put_phdr (&t, elfclass, PT_GNU_PROPERTY, seg_off, 12 + 4 + 256,
	    note_align (elfclass));
  put_nhdr (&t, 4, 0xfffffff8, NT_GNU_PROPERTY_TYPE_0);
  bput (&t, "GNU", 4);
  static const unsigned char zeros[256];
  bput (&t, zeros, sizeof (zeros));
  char *name = xasprintf ("r1-descsz%d", elfclass == ELFCLASS64 ? 64 : 32);
  run_case (name, &t);
  free (name);
}

/* Crafted n_namesz and n_descsz advance the note offset by 4 GiB while
   the loop guard in process_elf32_file, which compares pointers
   truncated to Elf32_Addr, still sees an offset within the segment.
   On a 64-bit ldconfig this only reproduces with an ELFCLASS32 object;
   the ELFCLASS64 variant just exercises the same fields in the other
   instance.  */
static void
case_r2_class (int elfclass)
{
  struct buf t = { 0 };
  size_t seg_off = ehdr_size (elfclass) + phdr_size (elfclass);
  put_ehdr (&t, elfclass, 1);
  put_phdr (&t, elfclass, PT_GNU_PROPERTY, seg_off, 16,
	    note_align (elfclass));
  put_nhdr (&t, 0x80000000, 0x7ffffff4, NT_GNU_PROPERTY_TYPE_0);
  static const unsigned char zeros[4];
  bput (&t, zeros, sizeof (zeros));	/* Rest of the 16-byte segment.  */
  char *name = xasprintf ("r2-class%d", elfclass == ELFCLASS64 ? 64 : 32);
  run_case (name, &t);
  free (name);
}

/* The segment ends after the first byte of the owner name
   (p_filesz == 13): the name and descriptor reads would run past the
   segment.  */
static void
case_r3_tail (int elfclass)
{
  struct buf t = { 0 };
  size_t seg_off = ehdr_size (elfclass) + phdr_size (elfclass);
  put_ehdr (&t, elfclass, 1);
  put_phdr (&t, elfclass, PT_GNU_PROPERTY, seg_off, 13,
	    note_align (elfclass));
  put_nhdr (&t, 4, 0xfffffff8, NT_GNU_PROPERTY_TYPE_0);
  bput (&t, "G", 1);		/* Last byte inside the segment.  */
  bput (&t, "NU", 3);		/* Past the segment, inside the file.  */
  static const unsigned char zeros[256];
  bput (&t, zeros, sizeof (zeros));
  char *name = xasprintf ("r3-tail%d", elfclass == ELFCLASS64 ? 64 : 32);
  run_case (name, &t);
  free (name);
}

/* The segment starts within the file but p_filesz runs past its
   end.  */
static void
case_r4_filesz (int elfclass)
{
  struct buf t = { 0 };
  size_t seg_off = ehdr_size (elfclass) + phdr_size (elfclass);
  put_ehdr (&t, elfclass, 1);
  put_phdr (&t, elfclass, PT_GNU_PROPERTY, seg_off, 0x10000,
	    note_align (elfclass));
  char *name = xasprintf ("r4-filesz%d", elfclass == ELFCLASS64 ? 64 : 32);
  run_case (name, &t);
  free (name);
}

/* The segment is too short to hold a note header.  */
static void
case_r5_short (int elfclass)
{
  struct buf t = { 0 };
  size_t seg_off = ehdr_size (elfclass) + phdr_size (elfclass);
  put_ehdr (&t, elfclass, 1);
  put_phdr (&t, elfclass, PT_GNU_PROPERTY, seg_off, 11,
	    note_align (elfclass));
  put_nhdr (&t, 4, 8, NT_GNU_PROPERTY_TYPE_0);
  char *name = xasprintf ("r5-short%d", elfclass == ELFCLASS64 ? 64 : 32);
  run_case (name, &t);
  free (name);
}

/* The owner name is present, but the padding needed to align the
   descriptor would run past the segment.  With p_filesz == 19 the
   padding check trips for both the 4- and the 8-byte alignment.  */
static void
case_r6_pad (int elfclass)
{
  struct buf t = { 0 };
  size_t seg_off = ehdr_size (elfclass) + phdr_size (elfclass);
  put_ehdr (&t, elfclass, 1);
  put_phdr (&t, elfclass, PT_GNU_PROPERTY, seg_off, 19,
	    note_align (elfclass));
  put_nhdr (&t, 5, 0, NT_GNU_PROPERTY_TYPE_0);
  bput (&t, "GNU\0X", 5);	/* n_namesz == 5.  */
  bput (&t, "pad", 3);		/* Bytes 17..18 inside the segment.  */
  char *name = xasprintf ("r6-pad%d", elfclass == ELFCLASS64 ? 64 : 32);
  run_case (name, &t);
  free (name);
}

/* A minimal well-formed object with dynamic metadata (DT_STRTAB,
   DT_SONAME) and well-formed notes for ldconfig to parse: a non-GNU
   note followed by a GNU note with a GNU_PROPERTY_X86_ISA_1_NEEDED
   property.  Built in the native class only.  */
static void
case_legit_prop (void)
{
  struct buf t = { 0 };
  int elfclass = sizeof (ElfW(Addr)) == 8 ? ELFCLASS64 : ELFCLASS32;
  size_t dyn_off = ehdr_size (elfclass) + 3 * phdr_size (elfclass);
  size_t strtab_off = dyn_off + 3 * dyn_size (elfclass);
  size_t seg_off = strtab_off + 32;
  /* One ISA property (8-byte header + 4-byte data), padded to a
     multiple of the address size.  */
  size_t descsz = addr_size (elfclass) == 8 ? 16 : 12;
  size_t seg_size = 16 + 16 + descsz;
  size_t file_size = seg_off + seg_size;

  put_ehdr (&t, elfclass, 3);
  put_phdr (&t, elfclass, PT_LOAD, 0, file_size, 0x1000);
  put_phdr (&t, elfclass, PT_DYNAMIC, dyn_off, 3 * dyn_size (elfclass),
	    note_align (elfclass));
  put_phdr (&t, elfclass, PT_GNU_PROPERTY, seg_off, seg_size,
	    note_align (elfclass));

  /* Dynamic entries: DT_STRTAB, DT_SONAME, DT_NULL.  */
  put_dyn (&t, elfclass, DT_STRTAB, strtab_off);
  put_dyn (&t, elfclass, DT_SONAME, 1);
  put_dyn (&t, elfclass, DT_NULL, 0);

  bput (&t, "\0libbadnote-legit.so", sizeof ("\0libbadnote-legit.so"));
  /* Pad up to the note segment.  */
  TEST_VERIFY_EXIT (t.n <= seg_off);
  memset (t.b + t.n, 0, seg_off - t.n);
  t.n = seg_off;

  /* A non-GNU note, to cover advancing to the next note.  */
  put_nhdr (&t, 3, 0, 1);
  bput (&t, "abc", 3);
  put_pad (&t, 12 + 3, note_align (elfclass));

  put_nhdr (&t, 4, descsz, NT_GNU_PROPERTY_TYPE_0);
  bput (&t, "GNU", 4);
  put_pad (&t, 12 + 4, note_align (elfclass));
  /* One GNU_PROPERTY_X86_ISA_1_NEEDED property, datasz 4.  */
  struct gnu_property p = { GNU_PROPERTY_X86_ISA_1_NEEDED, 4 };
  bput (&t, &p, sizeof p);
  uint32_t isa_1_needed = 1U << 2;	/* x86-64-v3.  */
  bput (&t, &isa_1_needed, sizeof isa_1_needed);
  put_pad (&t, 8 + 4, addr_size (elfclass));

  run_case ("legit-prop", &t);
}

static int
do_test (void)
{
  int native = sizeof (ElfW(Addr)) == 8 ? ELFCLASS64 : ELFCLASS32;
  int other = sizeof (ElfW(Addr)) == 8 ? ELFCLASS32 : ELFCLASS64;

  case_r0_offset (native);
  case_r0_offset (other);
  case_r1_descsz (native);
  case_r1_descsz (other);
  case_r2_class (native);
  case_r2_class (other);
  case_r3_tail (native);
  case_r3_tail (other);
  case_r4_filesz (native);
  case_r5_short (native);
  case_r6_pad (native);
  case_legit_prop ();
  return 0;
}

#include <support/test-driver.c>
