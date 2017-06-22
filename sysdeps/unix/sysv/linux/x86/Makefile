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

ifeq ($(subdir),setjmp)
tests += tst-saved_mask-1
endif

ifeq ($(enable-cet),yes)
ifeq ($(subdir),elf)
sysdep-dl-routines += dl-cet
endif

# Add -fcf-protection -mcet to CFLAGS when CET is enabled.
CFLAGS-.o += -fcf-protection -mcet
CFLAGS-.os += -fcf-protection -mcet
CFLAGS-.op += -fcf-protection -mcet
CFLAGS-.oS += -fcf-protection -mcet

# Compile assembly codes with <cet.h> when CET is enabled.
asm-CPPFLAGS += -fcf-protection -mcet -include cet.h

ifeq ($(subdir),elf)
ifeq (yes,$(build-shared))
tests-special += $(objpfx)check-cet.out
endif

# FIXME: Can't use all-built-dso in elf/Makefile since this file is
# processed before elf/Makefile.  Duplicate it here.
cet-built-dso := $(common-objpfx)elf/ld.so $(common-objpfx)libc.so \
		 $(filter-out $(common-objpfx)linkobj/libc.so, \
			      $(sort $(wildcard $(addprefix $(common-objpfx), \
							    */lib*.so \
							    iconvdata/*.so))))

$(cet-built-dso:=.note): %.note: %
	@rm -f $@T
	LC_ALL=C $(READELF) -n $< > $@T
	test -s $@T
	mv -f $@T $@
common-generated += $(cet-built-dso:$(common-objpfx)%=%.note)

$(objpfx)check-cet.out: $(..)sysdeps/unix/sysv/linux/x86/check-cet.awk \
			$(cet-built-dso:=.note)
	LC_ALL=C $(AWK) -f $^ > $@; \
	$(evaluate-test)
generated += check-cet.out
endif
endif
