ifeq ($(subdir),string)
sysdep_routines += memcpy_neon memcpy_vfp memchr_neon memcpy_arm

# For ifunc resolvers compiler might place the alias from generic
# symbol-hacks.h before the resolver implementation itself.
CFLAGS-.os += #-DNO_MEM_INTERAL_SYM_HACKS
endif
