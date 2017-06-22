# We don't need any header files.
abi-includes :=

abi-variants := 32 64 x32

abi-32-condition := !defined __x86_64__
abi-64-condition := defined __x86_64__ && defined __LP64__
abi-x32-condition := defined __x86_64__ && defined __ILP32__

ifeq ($(subdir),misc)
sysdep_headers += sys/elf.h sys/perm.h sys/reg.h sys/vm86.h sys/debugreg.h sys/io.h
endif

ifeq ($(subdir),nptl)
libpthread-sysdep_routines += elision-lock elision-unlock elision-timed \
			      elision-trylock
endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
endif

ifeq ($(enable-cet),yes)
ifeq ($(subdir),elf)
sysdep_routines += dl-cet-nonshared
sysdep-dl-routines += dl-cet
endif

# Add -fcf-protection -mcet to CFLAGS when CET is enabled.
CFLAGS-.o += -fcf-protection -mcet
CFLAGS-.os += -fcf-protection -mcet
CFLAGS-.op += -fcf-protection -mcet
CFLAGS-.oS += -fcf-protection -mcet

# Compile assembly codes with <cet.h> when CET is enabled.
asm-CPPFLAGS += -fcf-protection -mcet -include cet.h
endif
