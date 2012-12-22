UBOOTAML_VERSION:=0648a177f3ae87445f02003d52eeff3687ac165c
UBOOTAML_SITE=git@github.com:j1nx/Amlogic-reff16-uboot.git
UBOOTAML_SITE_METHOD=git
UBOOTAML_INSTALL_STAGING=YES
UBOOTAML_DEPENDENCIES=ubootamltoolchain

define UBOOTAML_BUILD_CMDS
	rm -f $(@D)/tools/ucl/libucl_linux.lib	
	rm -rf $(@D)/tools/ucl/ucl_cygwin_prj/ucl_linux
	$(MAKE) -C $(@D)/tools/ucl/ucl_cygwin_prj/
        cp -f $(@D)/tools/ucl/ucl_cygwin_prj/ucl_linux/libucl_linux.a $(@D)/tools/ucl/libucl_linux.lib
        $(MAKE) -C $(@D) m3_mbox_config
        PATH="$(TOOLCHAIN_EXTERNAL_DIR)/bin:$(TARGET_PATH)" $(MAKE) -C $(@D)
endef

define UBOOTAML_INSTALL_STAGING_CMDS
	cp -f $(@D)/build/u-boot-aml-ucl.bin $(BINARIES_DIR)/spi_M3_512.bin
        mkdir -p $(HOST_DIR)/usr/bin
	install $(@D)/build/tools/mkimage $(HOST_DIR)/usr/bin/mkimage
endef

$(eval $(call GENTARGETS,package/amlogic,ubootaml))
