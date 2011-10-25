#############################################################
#
#amljpeg
#
#############################################################
AMLJPEG_VERSION:=0.9.9

AMLJPEG_SOURCE=amljpeg-$(AMLJPEG_VERSION).tar.gz
AMLJPEG_SITE=./package/amlogic/amljpeg/src
AMLJPEG_SITE_METHOD=cp

define AMLJPEG_BUILD_CMDS
        $(MAKE) CC="$(TARGET_CC)" STAGING_DIR=$(STAGING_DIR) -C $(@D)
endef

define AMLJPEG_INSTALL_TARGET_CMDS
        install -m 755 $(@D)/libamljpeg.a $(TARGET_DIR)/usr/lib
endef

$(eval $(call GENTARGETS,package/amlogic,amljpeg))
