#############################################################
#
# amcodec
#
#############################################################
AMCODEC_VERSION:=0.9.9
AMCODEC_SOURCE=amcodec-$(AMCODEC_VERSION).tar.gz
AMCODEC_SITE=./package/amlogic/libplayer/src/amcodec/
AMCODEC_SITE_METHOD=cp

define AMCODEC_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define AMCODEC_INSTALL_TARGET_CMDS
	install -m 755 $(@D)/libamcodec.so.0.0 $(TARGET_DIR)/usr/lib
endef

$(eval $(call GENTARGETS,package/amlogic,amcodec))
