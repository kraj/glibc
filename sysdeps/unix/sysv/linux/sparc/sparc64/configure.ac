GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/unix/sysv/linux/sparc/sparc64.

LIBC_SLIBDIR_RTLDDIR([lib64], [lib64])

dnl Clang does not support -fcall-used-<reg>.
LIBC_TRY_CC_AND_TEST_CC_OPTION([for -fcall-used-g6],
  [-Werror -fcall-used-g6],
  libc_cv_cc_fcall_used_g6,
  [libc_cv_cc_fcall_used_g6=-fcall-used-g6],
  [libc_cv_cc_fcall_used_g6=],
  libc_cv_test_cc_fcall_used_g6,
  [libc_cv_test_cc_fcall_used_g6=-fcall-used-g6],
  [libc_cv_test_cc_fcall_used_g6=])
LIBC_CONFIG_VAR([config-cflags-fcall-used-g6],
		[$libc_cv_cc_fcall_used_g6])
