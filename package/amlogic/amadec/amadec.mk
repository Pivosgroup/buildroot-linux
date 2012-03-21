#############################################################
#
# amadec
#
#############################################################
AMADEC_VERSION:=0.9.9
AMADEC_SOURCE=amadec-$(AMADEC_VERSION).tar.gz
AMADEC_SITE=./package/amlogic/libplayer/src/amadec/
AMADEC_SITE_METHOD=cp
AMADEC_INSTALL_STAGING=YES

ifeq ($(BR2_PACKAGE_AMADEC),y)
AMADEC_DEPENDENCIES = alsa-lib
endif

define AMADEC_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define AMADEC_INSTALL_STAGING_CMDS
	cp -rf $(@D)/include/* $(STAGING_DIR)/usr/include
	install -m 755 $(@D)/libamadec.so* $(STAGING_DIR)/usr/lib
endef

define AMADEC_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/firmware
	cp -rf $(@D)/firmware/*.bin $(TARGET_DIR)/lib/firmware
	install -m 755 $(@D)/libamadec.so* $(TARGET_DIR)/usr/lib
endef

$(eval $(call GENTARGETS,package/amlogic,amadec))
