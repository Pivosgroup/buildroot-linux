#############################################################
#
# amcodec
#
#############################################################
AMCODEC_VERSION:=0.9.9
AMCODEC_SOURCE=amcodec-$(AMCODEC_VERSION).tar.gz
AMCODEC_SITE=./package/amlogic/libplayer/src/amcodec/
AMCODEC_SITE_METHOD=cp
AMCODEC_INSTALL_STAGING=YES
AMCODEC_DEPENDENCIES=amadec

define AMCODEC_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
endef

define AMCODEC_INSTALL_STAGING_CMDS
	install -m 644 $(@D)/libamcodec.so.0.0 $(STAGING_DIR)/usr/lib
	cd $(STAGING_DIR)/usr/lib; ln -sf libamcodec.so.0.0 libamcodec.so
	mkdir -p $(STAGING_DIR)/usr/include/amlplayer
	cp -rf $(@D)/include/* $(STAGING_DIR)/usr/include/amlplayer

	#temporary, until we sync with mainline xbmc
	cp -rf $(@D)/include/* $(STAGING_DIR)/usr/include
endef

define AMCODEC_INSTALL_TARGET_CMDS
	install -m 644 $(@D)/libamcodec.so.0.0 $(TARGET_DIR)/usr/lib
endef

$(eval $(call GENTARGETS,package/amlogic,amcodec))
