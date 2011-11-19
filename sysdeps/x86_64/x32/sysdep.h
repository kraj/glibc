/* Assembler macros for x32.
   Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#undef ASM_ADDR
#undef LP_SIZE

#undef RAX_LP
#undef RBP_LP
#undef RBX_LP
#undef RCX_LP
#undef RDI_LP
#undef RDX_LP
#undef RSP_LP
#undef RSI_LP
#undef R8_LP
#undef R9_LP
#undef R10_LP
#undef R11_LP
#undef R12_LP
#undef R13_LP
#undef R14_LP
#undef R15_LP

#ifdef	__ASSEMBLER__

# define LP_SIZE 4

# undef MOVE_LP
# define MOVE_LP movl

# undef ADD_LP
# define ADD_LP addl

# undef SUB_LP
# define SUB_LP subl

# undef CMP_LP
# define CMP_LP cmpl

# define ASM_ADDR .long

# define RAX_LP	eax
# define RBP_LP	ebp
# define RBX_LP	ebx
# define RCX_LP	ecx
# define RDI_LP	edi
# define RDX_LP	edx
# define RSI_LP	esi
# define RSP_LP	esp
# define R8_LP	r8d
# define R9_LP	r9d
# define R10_LP	r10d
# define R11_LP	r11d
# define R12_LP	r12d
# define R13_LP	r13d
# define R14_LP	r14d
# define R15_LP	r15d

#else	/* __ASSEMBLER__ */

# define LP_SIZE "4"

# define ASM_ADDR ".long"

# define RAX_LP	"eax"
# define RBP_LP	"ebp"
# define RBX_LP	"ebx"
# define RCX_LP	"ecx"
# define RDI_LP	"edi"
# define RDX_LP	"edx"
# define RSI_LP	"esi"
# define RSP_LP	"esp"
# define R8_LP	"r8d"
# define R9_LP	"r9d"
# define R10_LP	"r10d"
# define R11_LP	"r11d"
# define R12_LP	"r12d"
# define R13_LP	"r13d"
# define R14_LP	"r14d"
# define R15_LP	"r15d"

#endif	/* __ASSEMBLER__ */
