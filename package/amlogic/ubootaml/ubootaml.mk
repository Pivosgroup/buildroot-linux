UBOOTAML_VERSION:=50dba3182b40ff71a68ddae1ba88a53cdab93935
UBOOTAML_SITE=git@github.com:Pivosgroup/buildroot-uboot.git
UBOOTAML_SITE_METHOD=git
UBOOTAML_INSTALL_STAGING=YES
UBOOTAML_DEPENDENCIES=ubootamltoolchain
TARGET_PATH:=$(TOOLCHAIN_EXTERNAL_DIR)/bin:$(TARGET_PATH)

define UBOOTAML_BUILD_CMDS
	rm -f $(@D)/tools/ucl/libucl_linux.lib
	$(MAKE) -C $(@D)/tools/ucl/ucl_cygwin_prj/
        cp -f $(@D)/tools/ucl/ucl_cygwin_prj/objs/libucl.a $(@D)/tools/ucl/libucl_linux.lib
        $(MAKE) -C $(@D) stv_mbx_m3_512_config
        PATH=$(TARGET_PATH) $(MAKE) -C $(@D)
endef

define UBOOTAML_INSTALL_STAGING_CMDS
	cp -f $(@D)/build/u-boot-aml-ucl.bin $(BINARIES_DIR)/spi_M3_512.bin
	install $(@D)/build/tools/mkimage $(HOST_DIR)/usr/bin
endef

$(eval $(call GENTARGETS,package/amlogic,ubootaml))
