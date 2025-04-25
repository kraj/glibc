/* Some compiler optimizations may transform loops into memset/memmove
   calls and without proper declaration it may generate PLT calls.  */
#if !defined __ASSEMBLER__ && IS_IN (libc) && defined SHARED \
    && !defined LIBC_NONSHARED
asm ("memmove = __GI_memmove");
asm ("memset = __GI_memset");
asm ("memcpy = __GI_memcpy");

/* Some targets do not use __stack_chk_fail_local.  In libc.so,
   redirect __stack_chk_fail to a hidden reference
   __stack_chk_fail_local, to avoid the PLT reference.
   __stack_chk_fail itself is a global symbol, exported from libc.so,
   and cannot be made hidden.  */

# if IS_IN (libc) && defined SHARED \
  && defined STACK_PROTECTOR_LEVEL && STACK_PROTECTOR_LEVEL > 0
asm (".hidden __stack_chk_fail_local\n"
     "__stack_chk_fail = __stack_chk_fail_local");
# endif
#endif

#if !defined __ASSEMBLER__ && IS_IN(rtld) && defined ENABLE_UBSAN \
  && !defined DISABLE_USAN_INTERNAL_REDIR
asm ("__ubsan_handle_negate_overflow = "
     "__GI___ubsan_handle_negate_overflow");
asm ("__ubsan_handle_shift_out_of_bounds = "
     "__GI___ubsan_handle_shift_out_of_bounds");
asm ("__ubsan_handle_divrem_overflow = "
     "__GI___ubsan_handle_divrem_overflow");
asm ("__ubsan_handle_vla_bound_not_positive = "
     "__GI___ubsan_handle_vla_bound_not_positive");
asm ("__ubsan_handle_pointer_overflow = "
     "__GI___ubsan_handle_pointer_overflow");
asm ("__ubsan_handle_load_invalid_value ="
     "__GI___ubsan_handle_load_invalid_value");
asm ("__ubsan_handle_out_of_bounds = "
     "__GI___ubsan_handle_out_of_bounds");
asm ("__ubsan_handle_sub_overflow = "
     "__GI___ubsan_handle_sub_overflow");
asm ("__ubsan_handle_add_overflow = "
     "__GI___ubsan_handle_add_overflow");
asm ("__ubsan_handle_mul_overflow = "
     "__GI___ubsan_handle_mul_overflow");
asm ("__ubsan_handle_type_mismatch_v1 = "
     "__GI___ubsan_handle_type_mismatch_v1");
asm ("__ubsan_handle_nonnull_return_v1 = "
     "__GI___ubsan_handle_nonnull_return_v1");
asm ("__ubsan_handle_nonnull_arg = "
     "__GI___ubsan_handle_nonnull_arg");
asm ("__ubsan_handle_invalid_builtin = "
     "__GI___ubsan_handle_invalid_builtin");
asm ("__ubsan_handle_builtin_unreachable = "
     "__GI___ubsan_handle_builtin_unreachable");
#endif
