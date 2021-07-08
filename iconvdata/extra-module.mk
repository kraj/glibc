mod := $(firstword $(extra-modules-left))
extra-modules-left := $(strip $(filter-out $(mod),$(extra-modules-left)))

extra-objs := $(extra-objs) $(patsubst %,%.os,$($(mod)-routines))

$(objpfx)$(mod).so: $(addprefix $(objpfx),$(addsuffix .os,$($(mod)-routines)))\
		    $(shlib-lds) $(link-libc-deps)
	$(build-module-asneeded)

ifneq ($(ld-zunique),yes)
$(objpfx)$(mod).so: $(common-objpfx)/elf/dynamic-notes.os
endif

ifneq (,$(extra-modules-left))
include extra-module.mk
endif
