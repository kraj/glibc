GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/sparc/sparc32

# Test if compiler defines __sparc_v9__.
AC_CACHE_CHECK([for sparcv9],
  [libc_sparcv9],
  [AC_EGREP_CPP(yes,[#ifdef __sparc_v9__
                      yes
                     #endif
  ], libc_sparcv9=yes, libc_sparcv9=no)])
if test $libc_sparcv9 = no; then
  AC_MSG_ERROR([no support for pre-v9 sparc])
fi
