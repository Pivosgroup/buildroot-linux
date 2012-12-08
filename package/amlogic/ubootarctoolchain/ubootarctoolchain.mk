UBOOTARCTOOLCHAIN_SITE=http://www.codesourcery.com/sgpp/lite/arm/portal/package6493/public/arm-none-eabi/
UBOOTARCTOOLCHAIN_VERSION:=arm-2010q1-188-arm-none-eabi-i686-pc-linux-gnu
UBOOTARCTOOLCHAIN_SOURCE:=$(UBOOTARCTOOLCHAIN_VERSION).tar.bz2

define UBOOTARCTOOLCHAIN_BUILD_CMDS
	cp -rf $(@D)/arm-none-eabi $(TOOLCHAIN_EXTERNAL_DIR)
	cp -rf $(@D)/bin $(TOOLCHAIN_EXTERNAL_DIR)
	cp -rf $(@D)/lib $(TOOLCHAIN_EXTERNAL_DIR)
	cp -rf $(@D)/libexec $(TOOLCHAIN_EXTERNAL_DIR)
	cp -rf $(@D)/share $(TOOLCHAIN_EXTERNAL_DIR)
endef

$(eval $(call GENTARGETS,package/amlogic,ubootarctoolchain)) 
