# ===========================================================================
# EGLIBC option-groups configuration targets
# These targets are included from top-level makefile

ifneq ($(kconfig_tools),)
ifneq (no,$(PERL))

ocdir := options-config

OconfigDefaults     := option-groups.defaults
OconfigDefaults_tmp := $(common-objpfx).tmp.defconfig
OconfigDef          := option-groups.def
Oconfig             := $(common-objpfx)option-groups.config
Oconfig_tmp         := $(common-objpfx).tmp.config

conf  := $(kconfig_tools)/conf
mconf := $(kconfig_tools)/mconf

preproc  := $(PERL) $(ocdir)/config-preproc.pl
postproc := $(PERL) $(ocdir)/config-postproc.pl

PHONY += defconfig config menuconfig

defconfig: $(conf) $(OconfigDefaults) $(OconfigDef)
	rm -f $(OconfigDefaults_tmp)
	rm -f $(Oconfig_tmp)
	$(preproc) $(OconfigDefaults) > $(OconfigDefaults_tmp)
	KCONFIG_CONFIG=$(Oconfig_tmp) $< --defconfig=$(OconfigDefaults_tmp) \
				$(OconfigDef)
	$(postproc) $(OconfigDefaults) $(Oconfig_tmp) > $(Oconfig)
	rm $(Oconfig_tmp)
	rm $(OconfigDefaults_tmp)

config: $(conf) $(OconfigDefaults) $(OconfigDef)
	rm -f $(Oconfig_tmp)
	$(preproc) $(wildcard $(Oconfig)) > $(Oconfig_tmp)
	KCONFIG_CONFIG=$(Oconfig_tmp) $< --oldaskconfig $(OconfigDef)
	$(postproc) $(OconfigDefaults) $(Oconfig_tmp) > $(Oconfig)
	rm $(Oconfig_tmp)

menuconfig: $(mconf) $(OconfigDefaults) $(OconfigDef)
	rm -f $(Oconfig_tmp)
	$(preproc) $(wildcard $(Oconfig)) > $(Oconfig_tmp)
	KCONFIG_CONFIG=$(Oconfig_tmp) $< $(OconfigDef)
	$(postproc) $(OconfigDefaults) $(Oconfig_tmp) > $(Oconfig)
	rm $(Oconfig_tmp)

# Help text used by make help
help:
	@echo  '  defconfig	  - New config with default from default config'
	@echo  '  config	  - Update current config utilising a line-oriented program'
	@echo  '  menuconfig	  - Update current config utilising a menu based program'

endif
endif
