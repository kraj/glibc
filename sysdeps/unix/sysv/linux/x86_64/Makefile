32bit-predefine = __i386__
64bit-predefine = __x86_64__ __LP64__

ifeq ($(subdir),misc)
sysdep_routines += ioperm iopl
sysdep_headers += sys/perm.h sys/reg.h sys/debugreg.h sys/io.h
endif

ifeq ($(subdir),stdlib)
sysdep_routines += __start_context
endif

ifeq ($(subdir),csu)
gen-as-const-headers += ucontext_i.sym
endif

ifeq ($(subdir),misc)
gen-as-const-headers += sigaltstack.sym
endif

ifeq ($(subdir),elf)
sysdep_routines += dl-vdso
endif

# Generate the list of SYS_* macros for the system calls (__NR_* macros).
# For bi-arch platforms, the CPU/Makefile defines {32,64}bit-predefine and
# we generate a file that uses <bits/wordsize.h>.
$(objpfx)syscall-%.h $(objpfx)syscall-%.d: ../sysdeps/unix/sysv/linux/sys/syscall.h
	$(make-target-directory)
	{ \
	 echo '/* Generated at libc build time from kernel syscall list.  */';\
	 echo ''; \
	 echo '#ifndef _SYSCALL_H'; \
	 echo '# error "Never use <bits/syscall.h> directly; include <sys/syscall.h> instead."'; \
	 echo '#endif'; \
	 echo ''; \
	 $(CC) -E -MD -MP -MF $(@:.h=.d)-t1 -MT '$(@:.d=.h) $(@:.h=.d)' \
	       -x c $(sysincludes) $< $(addprefix -U,$(64bit-predefine)) \
	       $(addprefix -D,$(32bit-predefine)) -D_LIBC -dM | \
	 sed -n 's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p' | \
	 LC_ALL=C sort > $(@:.d=.h).new32; \
	 $(CC) -E -MD -MP -MF $(@:.h=.d)-t2 -MT '$(@:.d=.h) $(@:.h=.d)' \
	       -x c $(sysincludes) $< $(addprefix -U,$(32bit-predefine)) \
	       $(addprefix -D,$(64bit-predefine)) -D_LIBC -dM | \
	       grep -i -v _x32_ | grep -v "COMMON_SYSCALL(n)" | \
	 sed -n 's@^#define __NR_\([^ ]*\) .*$$@#define SYS_\1 __NR_\1@p' | \
	 LC_ALL=C sort > $(@:.d=.h).new64; \
	 if cmp -s $(@:.d=.h).new32 $(@:.d=.h).new64; then \
	   cat $(@:.d=.h).new32; \
	 else \
	   echo ''; \
	   LC_ALL=C comm -12 $(@:.d=.h).new32 $(@:.d=.h).new64; \
	   echo '#ifdef __x86_64__'; \
	   LC_ALL=C comm -13 $(@:.d=.h).new32 $(@:.d=.h).new64; \
	   echo '#else'; \
	   LC_ALL=C comm -23 $(@:.d=.h).new32 $(@:.d=.h).new64; \
	   echo '#endif'; \
	 fi; \
	 rm -f $(@:.d=.h).new32 $(@:.d=.h).new64; \
	} > $(@:.d=.h).new
	mv -f $(@:.d=.h).new $(@:.d=.h)
ifneq (,$(objpfx))
	sed $(sed-remove-objpfx) $(@:.h=.d)-t1 $(@:.h=.d)-t2 > $(@:.h=.d)-t3
else
	cat $(@:.h=.d)-t1 $(@:.h=.d)-t2 > $(@:.h=.d)-t3
endif
	rm -f $(@:.h=.d)-t1 $(@:.h=.d)-t2
	mv -f $(@:.h=.d)-t3 $(@:.h=.d)
