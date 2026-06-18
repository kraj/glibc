libmvec-supported-funcs = acos \
                          acosh \
                          acospi \
                          asin \
                          asinh \
                          asinpi \
                          atan \
                          atanh \
                          atanpi \
                          atan2 \
                          atan2pi \
                          cbrt \
                          cos \
                          cosh \
                          cospi \
                          erf \
                          erfc \
                          exp \
                          exp10 \
                          exp2 \
                          expm1 \
                          exp2m1 \
                          exp10m1 \
                          hypot \
                          log \
                          log10 \
                          log10p1 \
                          log1p \
                          log2 \
                          log2p1 \
                          pow \
                          powr \
                          rsqrt \
                          sin \
                          sinh \
                          sinpi \
                          tan \
                          tanh \
                          tanpi

float-advsimd-funcs = $(libmvec-supported-funcs)
double-advsimd-funcs = $(libmvec-supported-funcs)
float-sve-funcs = $(libmvec-supported-funcs)
double-sve-funcs = $(libmvec-supported-funcs)

ifeq ($(subdir),mathvec)
generated-dirs += advsimd sve
generated += advsimd/stamp sve/stamp
before-compile += $(addprefix $(objpfx),advsimd/stamp sve/stamp)

$(objpfx)advsimd/stamp $(objpfx)sve/stamp:
	$(make-target-directory)
	touch $@

sysd-rules-patterns += advsimd/%:advsimd/% sve/%:sve/%

libmvec-support = $(addprefix advsimd/,$(addsuffix f,$(float-advsimd-funcs))) \
                  $(addprefix advsimd/,$(double-advsimd-funcs)) \
                  $(addprefix sve/,$(addsuffix f,$(float-sve-funcs))) \
                  $(addprefix sve/,$(double-sve-funcs)) \
                  v_log_data \
                  v_exp_data \
                  v_log2_data \
                  v_log10_data \
                  erf_data \
                  erff_data \
                  v_exp_tail_data \
                  erfc_data \
                  erfcf_data \
                  v_pow_exp_data \
                  v_pow_log_data \
                  v_powf_data
endif

# Enable SVE for building libmvec.  Since CFLAGS may contain a -mcpu or -march,
# add a generic -mcpu and -march with SVE enabled.  Also use a tune for a modern
# SVE core.

sve-cflags = -mcpu=generic+sve -march=armv8-a+sve -mtune=neoverse-v1

ifeq ($(build-mathvec),yes)
bench-libmvec = $(addprefix float-advsimd-,$(float-advsimd-funcs)) \
                $(addprefix double-advsimd-,$(double-advsimd-funcs)) \
                $(addprefix float-sve-,$(float-sve-funcs)) \
                $(addprefix double-sve-,$(double-sve-funcs))
endif

$(objpfx)bench-float-advsimd-%.c:
	$(PYTHON) $(..)sysdeps/aarch64/fpu/scripts/bench_libmvec_advsimd.py $(basename $(@F)) > $@
$(objpfx)bench-double-advsimd-%.c:
	$(PYTHON) $(..)sysdeps/aarch64/fpu/scripts/bench_libmvec_advsimd.py $(basename $(@F)) > $@
$(objpfx)bench-float-sve-%.c:
	$(PYTHON) $(..)sysdeps/aarch64/fpu/scripts/bench_libmvec_sve.py $(basename $(@F)) > $@
$(objpfx)bench-double-sve-%.c:
	$(PYTHON) $(..)sysdeps/aarch64/fpu/scripts/bench_libmvec_sve.py $(basename $(@F)) > $@

ifeq (${STATIC-BENCHTESTS},yes)
libmvec-benchtests = $(common-objpfx)mathvec/libmvec.a $(common-objpfx)math/libm.a
else
libmvec-benchtests = $(libmvec) $(libm)
endif

$(addprefix $(objpfx)bench-,$(bench-libmvec)): $(libmvec-benchtests)

ifeq ($(build-mathvec),yes)
libmvec-tests += float-advsimd double-advsimd float-sve double-sve
endif

define sve-bench-float-cflags-template
CFLAGS-bench-float-sve-$(1).c += $(sve-cflags)
endef

define sve-bench-double-cflags-template
CFLAGS-bench-double-sve-$(1).c += $(sve-cflags)
endef

$(foreach f,$(float-sve-funcs), $(eval $(call sve-bench-float-cflags-template,$(f))))
$(foreach f,$(double-sve-funcs), $(eval $(call sve-bench-double-cflags-template,$(f))))

# Specialize CFLAGS used for building SVE sources.
define sve-object-cflags-template
$(objpfx)sve/%$(1): CFLAGS += $(sve-cflags)
endef

$(foreach o,$(all-object-suffixes),$(eval $(call sve-object-cflags-template,$(o))))

CFLAGS-test-float-sve-wrappers.c = $(sve-cflags)
CFLAGS-test-double-sve-wrappers.c = $(sve-cflags)
