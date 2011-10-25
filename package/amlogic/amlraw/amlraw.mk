#############################################################
#
#amlraw
#
#############################################################
AMLRAW_VERSION:=0.9.9
AMLRAW_SOURCE=amlraw-$(AMADEC_VERSION).tar.gz
AMLRAW_SITE=./package/amlogic/amlraw/src
AMLRAW_SITE_METHOD=cp
AMLRAW_INSTALL_STAGING=YES

ifeq ($(BR2_PACKAGE_AMLRAW),y)
AMLRAW_DEPENDENCIES = alsa-lib
endif

define AMLRAW_BUILD_CMDS
        $(MAKE) CC="$(TARGET_CC)" STAGING_DIR=$(STAGING_DIR) TARGET_DIR=$(STAGING_DIR) -C $(@D) libamlraw.a
endef

define AMLRAW_INSTALL_STAGING_CMDS
        install -m 755 $(@D)/libamlraw.a $(STAGING_DIR)/usr/lib
endef

$(eval $(call GENTARGETS,package/amlogic,amlraw))
