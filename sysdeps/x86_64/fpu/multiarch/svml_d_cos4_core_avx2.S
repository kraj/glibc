/* Function cos vectorized with AVX2.
   Copyright (C) 2014-2025 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include "svml_d_trig_data.h"

	.section .text.avx2, "ax", @progbits
ENTRY (_ZGVdN4v_cos_avx2)

/* ALGORITHM DESCRIPTION:

      ( low accuracy ( < 4ulp ) or enhanced performance
       ( half of correct mantissa ) implementation )

      Argument representation:
      arg + Pi/2 = (N*Pi + R)

      Result calculation:
      cos(arg) = sin(arg+Pi/2) = sin(N*Pi + R) = (-1)^N * sin(R)
      sin(R) is approximated by corresponding polynomial
 */
        pushq     %rbp
        cfi_adjust_cfa_offset (8)
        cfi_rel_offset (%rbp, 0)
        movq      %rsp, %rbp
        cfi_def_cfa_register (%rbp)
        andq      $-64, %rsp
        subq      $448, %rsp
        movq      __svml_d_trig_data@GOTPCREL(%rip), %rax
        vmovapd   %ymm0, %ymm1
        vmovupd __dInvPI(%rax), %ymm4
        vmovupd __dRShifter(%rax), %ymm5

/*
   ARGUMENT RANGE REDUCTION:
   Add Pi/2 to argument: X' = X+Pi/2
 */
        vaddpd __dHalfPI(%rax), %ymm1, %ymm7

/* Get absolute argument value: X' = |X'| */
        vandpd __dAbsMask(%rax), %ymm7, %ymm2

/* Y = X'*InvPi + RS : right shifter add */
        vfmadd213pd %ymm5, %ymm4, %ymm7
        vmovupd __dC7(%rax), %ymm4

/* Check for large arguments path */
        vcmpnle_uqpd __dRangeVal(%rax), %ymm2, %ymm3

/* N = Y - RS : right shifter sub */
        vsubpd    %ymm5, %ymm7, %ymm6
        vmovupd __dPI1_FMA(%rax), %ymm2

/* SignRes = Y<<63 : shift LSB to MSB place for result sign */
        vpsllq    $63, %ymm7, %ymm7

/* N = N - 0.5 */
        vsubpd __dOneHalf(%rax), %ymm6, %ymm0
        vmovmskpd %ymm3, %ecx

/* R = X - N*Pi1 */
        vmovapd   %ymm1, %ymm3
        vfnmadd231pd %ymm0, %ymm2, %ymm3

/* R = R - N*Pi2 */
        vfnmadd231pd __dPI2_FMA(%rax), %ymm0, %ymm3

/* R = R - N*Pi3 */
        vfnmadd132pd __dPI3_FMA(%rax), %ymm3, %ymm0

/* POLYNOMIAL APPROXIMATION: R2 = R*R */
        vmulpd    %ymm0, %ymm0, %ymm5
        vfmadd213pd __dC6(%rax), %ymm5, %ymm4
        vfmadd213pd __dC5(%rax), %ymm5, %ymm4
        vfmadd213pd __dC4(%rax), %ymm5, %ymm4

/* Poly = C3+R2*(C4+R2*(C5+R2*(C6+R2*C7))) */
        vfmadd213pd __dC3(%rax), %ymm5, %ymm4

/* Poly = R+R*(R2*(C1+R2*(C2+R2*Poly))) */
        vfmadd213pd __dC2(%rax), %ymm5, %ymm4
        vfmadd213pd __dC1(%rax), %ymm5, %ymm4
        vmulpd    %ymm5, %ymm4, %ymm6
        vfmadd213pd %ymm0, %ymm0, %ymm6

/*
   RECONSTRUCTION:
   Final sign setting: Res = Poly^SignRes */
        vxorpd    %ymm7, %ymm6, %ymm0
        testl     %ecx, %ecx
        jne       .LBL_1_3

.LBL_1_2:
        cfi_remember_state
        movq      %rbp, %rsp
        cfi_def_cfa_register (%rsp)
        popq      %rbp
        cfi_adjust_cfa_offset (-8)
        cfi_restore (%rbp)
        ret

.LBL_1_3:
        cfi_restore_state
        vmovupd   %ymm1, 320(%rsp)
        vmovupd   %ymm0, 384(%rsp)
        je        .LBL_1_2

        xorb      %dl, %dl
        xorl      %eax, %eax
        vmovups   %ymm8, 224(%rsp)
        vmovups   %ymm9, 192(%rsp)
        vmovups   %ymm10, 160(%rsp)
        vmovups   %ymm11, 128(%rsp)
        vmovups   %ymm12, 96(%rsp)
        vmovups   %ymm13, 64(%rsp)
        vmovups   %ymm14, 32(%rsp)
        vmovups   %ymm15, (%rsp)
        movq      %rsi, 264(%rsp)
        movq      %rdi, 256(%rsp)
        movq      %r12, 296(%rsp)
        cfi_offset_rel_rsp (12, 296)
        movb      %dl, %r12b
        movq      %r13, 288(%rsp)
        cfi_offset_rel_rsp (13, 288)
        movl      %ecx, %r13d
        movq      %r14, 280(%rsp)
        cfi_offset_rel_rsp (14, 280)
        movl      %eax, %r14d
        movq      %r15, 272(%rsp)
        cfi_offset_rel_rsp (15, 272)
        cfi_remember_state

.LBL_1_6:
        btl       %r14d, %r13d
        jc        .LBL_1_12

.LBL_1_7:
        lea       1(%r14), %esi
        btl       %esi, %r13d
        jc        .LBL_1_10

.LBL_1_8:
        incb      %r12b
        addl      $2, %r14d
        cmpb      $16, %r12b
        jb        .LBL_1_6

        vmovups   224(%rsp), %ymm8
        vmovups   192(%rsp), %ymm9
        vmovups   160(%rsp), %ymm10
        vmovups   128(%rsp), %ymm11
        vmovups   96(%rsp), %ymm12
        vmovups   64(%rsp), %ymm13
        vmovups   32(%rsp), %ymm14
        vmovups   (%rsp), %ymm15
        vmovupd   384(%rsp), %ymm0
        movq      264(%rsp), %rsi
        movq      256(%rsp), %rdi
        movq      296(%rsp), %r12
        cfi_restore (%r12)
        movq      288(%rsp), %r13
        cfi_restore (%r13)
        movq      280(%rsp), %r14
        cfi_restore (%r14)
        movq      272(%rsp), %r15
        cfi_restore (%r15)
        jmp       .LBL_1_2

.LBL_1_10:
        cfi_restore_state
        movzbl    %r12b, %r15d
        shlq      $4, %r15
        vmovsd    328(%rsp,%r15), %xmm0
        vzeroupper

        call      JUMPTARGET(cos)

        vmovsd    %xmm0, 392(%rsp,%r15)
        jmp       .LBL_1_8

.LBL_1_12:
        movzbl    %r12b, %r15d
        shlq      $4, %r15
        vmovsd    320(%rsp,%r15), %xmm0
        vzeroupper

        call      JUMPTARGET(cos)

        vmovsd    %xmm0, 384(%rsp,%r15)
        jmp       .LBL_1_7

END (_ZGVdN4v_cos_avx2)
