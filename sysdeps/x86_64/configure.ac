GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/x86_64.

dnl Check if asm supports AVX512DQ.
AC_CACHE_CHECK(for AVX512DQ support in assembler, libc_cv_asm_avx512dq, [dnl
cat > conftest.s <<\EOF
        vandpd (%rax), %zmm6, %zmm1
EOF
if AC_TRY_COMMAND(${CC-cc} -c $ASFLAGS conftest.s 1>&AS_MESSAGE_LOG_FD); then
  libc_cv_asm_avx512dq=yes
else
  libc_cv_asm_avx512dq=no
fi
rm -f conftest*])
if test $libc_cv_asm_avx512dq = yes; then
  AC_DEFINE(HAVE_AVX512DQ_ASM_SUPPORT)
fi

dnl Check if -mavx512f works.
AC_CACHE_CHECK(for AVX512 support, libc_cv_cc_avx512, [dnl
LIBC_TRY_CC_OPTION([-mavx512f], [libc_cv_cc_avx512=$libc_cv_asm_avx512dq], [libc_cv_cc_avx512=no])
])
if test $libc_cv_cc_avx512 = yes; then
  AC_DEFINE(HAVE_AVX512_SUPPORT)
fi
LIBC_CONFIG_VAR([config-cflags-avx512], [$libc_cv_cc_avx512])

dnl Check whether asm supports Intel MPX
AC_CACHE_CHECK(for Intel MPX support, libc_cv_asm_mpx, [dnl
cat > conftest.s <<\EOF
        bndmov %bnd0,(%rsp)
EOF
if AC_TRY_COMMAND(${CC-cc} -c $ASFLAGS conftest.s 1>&AS_MESSAGE_LOG_FD); then
  libc_cv_asm_mpx=yes
else
  libc_cv_asm_mpx=no
fi
rm -f conftest*])
if test $libc_cv_asm_mpx = yes; then
  AC_DEFINE(HAVE_MPX_SUPPORT)
fi

dnl Check for ___tls_get_addr.
AC_CACHE_CHECK(for ___tls_get_addr,
	       libc_cv_cc____tls_get_addr, [dnl
cat > conftest.c <<\EOF
__thread int x;
int
_start (void)
{
  return x;
}
EOF
if AC_TRY_COMMAND(${CC-cc} $CFLAGS $CPPFLAGS -O2 $no_ssp -fPIC -c conftest.c) \
  && AC_TRY_COMMAND($NM -u conftest.o | grep -q "U ___tls_get_addr"); then
  libc_cv_cc____tls_get_addr=yes
else
  libc_cv_cc____tls_get_addr=no
fi
rm -f conftest*])

if test $libc_cv_cc____tls_get_addr = no; then
  dnl Check if __tls_get_addr -> ___tls_get_addr redirection works.
  AC_CACHE_CHECK(for __tls_get_addr -> ___tls_get_addr redirection,
		 libc_cv____tls_get_addr, [dnl
cat > conftest.c <<\EOF
__thread int x;
asm (".weakref __tls_get_addr, ___tls_get_addr");

int
_start (void)
{
  return x;
}
EOF
if AC_TRY_COMMAND(${CC-cc} $CFLAGS $CPPFLAGS -O2 $no_ssp -fPIC -c conftest.c) \
  && AC_TRY_COMMAND($NM -u conftest.o | grep -q "U ___tls_get_addr") \
  && $OBJDUMP -dw conftest.o | grep -E "sub[[lq]]? +\\\$0x8,%[[re]]sp" 1>&AS_MESSAGE_LOG_FD \
  && AC_TRY_COMMAND(${CC-cc} $LDFLAGS -nostdlib -nostartfiles conftest.o -o conftest); then
  libc_cv____tls_get_addr=yes
else
  libc_cv____tls_get_addr=no
fi
rm -f conftest*])
else
  libc_cv____tls_get_addr=no
fi
LIBC_CONFIG_VAR([have-___tls_get_addr], [$libc_cv____tls_get_addr])

if test x"$build_mathvec" = xnotset; then
  build_mathvec=yes
fi

dnl It is always possible to access static and hidden symbols in an
dnl position independent way.
AC_DEFINE(PI_STATIC_AND_HIDDEN)

test -n "$critic_missing" && AC_MSG_ERROR([
*** $critic_missing])
