#############################################################
#
# amadec
#
#############################################################
AMADEC_VERSION:=0.9.9
AMADEC_SOURCE=amadec-$(AMADEC_VERSION).tar.gz
AMADEC_SITE=./package/amlogic/libplayer/src/amadec/
AMADEC_SITE_METHOD=cp

ifeq ($(BR2_PACKAGE_AMADEC),y)
AMADEC_DEPENDENCIES = alsa-lib
endif

define AMADEC_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define AMADEC_INSTALL_TARGET_CMDS
        install -m 755 $(@D)/amadecc $(TARGET_DIR)/usr/bin
        install -m 755 $(@D)/amadecd $(TARGET_DIR)/usr/bin
endef

$(eval $(call GENTARGETS,package/amlogic,amadec))
