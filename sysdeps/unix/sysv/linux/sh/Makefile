ifeq ($(subdir),io)
sysdep_routines += pipe
endif

ifeq ($(subdir),stdlib)
gen-as-const-headers += ucontext_i.sym
endif

# <sigaltstack-offsets.h> is only used by ____longjmp_chk.S, which is
# built in the debug subdirectory.
ifeq ($(subdir),debug)
gen-as-const-headers += sigaltstack-offsets.sym
endif

ifeq ($(subdir),math)
# The libm.so link can't find __fpscr_values
libm.so-no-z-defs = yes
endif
