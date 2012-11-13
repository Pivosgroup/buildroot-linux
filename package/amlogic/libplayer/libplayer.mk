#############################################################
#
# libplayer
#
#############################################################
LIBPLAYER_VERSION:=0.9.9
LIBPLAYER_SOURCE=libplayer-$(AMADEC_VERSION).tar.gz
LIBPLAYER_SITE=./package/amlogic/libplayer/src/
LIBPLAYER_INSTALL_STAGING=YES
LIBPLAYER_INSTALL_TARGET=YES
LIBPLAYER_SITE_METHOD=cp

ifeq ($(BR2_PACKAGE_LIBPLAYER),y)
LIBPLAYER_DEPENDENCIES += alsa-lib librtmp pkg-config
endif

AMFFMPEG_DIR=$(BUILD_DIR)/libplayer-$(LIBPLAYER_VERSION)/amffmpeg

define LIBPLAYER_BUILD_CMDS
 $(call AMFFMPEG_CONFIGURE_CMDS)
 $(call AMFFMPEG_BUILD_CMDS)
 $(call AMFFMPEG_INSTALL_STAGING_CMDS)
 $(call AMFFMPEG_STAGING_AMFFMPEG_EXTRA_HEADERS)

 mkdir -p $(STAGING_DIR)/usr/include/amlplayer
 $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" HEADERS_DIR="$(STAGING_DIR)/usr/include/amlplayer" \
  CROSS_PREFIX="$(TARGET_CROSS)" SYSROOT="$(STAGING_DIR)" PREFIX="$(STAGING_DIR)/usr" -C $(@D)/amadec install
 $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" HEADERS_DIR="$(STAGING_DIR)/usr/include/amlplayer" CROSS_PREFIX="$(TARGET_CROSS)" \
  SYSROOT="$(STAGING_DIR)" PREFIX="$(STAGING_DIR)/usr" SRC=$(BUILD_DIR)/libplayer-$(LIBPLAYER_VERSION)/amcodec -C $(@D)/amcodec install
 $(MAKE) CROSS="$(TARGET_CROSS)" CC="$(TARGET_CC)" LD="$(TARGET_LD)" PREFIX="$(STAGING_DIR)/usr" \
  SRC="$(BUILD_DIR)/libplayer-$(LIBPLAYER_VERSION)/amplayer" -C $(@D)/amplayer
endef

define LIBPLAYER_INSTALL_STAGING_CMDS
 $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" INSTALL_DIR="$(STAGING_DIR)/usr/lib" STAGING="$(STAGING_DIR)/usr" -C $(@D)/amplayer install

 #temporary, until we sync with mainline xbmc
 cp -rf $(BUILD_DIR)/libplayer-$(LIBPLAYER_VERSION)/amcodec/include/* $(STAGING_DIR)/usr/include
endef

define LIBPLAYER_INSTALL_TARGET_CMDS
 $(call AMFFMPEG_INSTALL_TARGET_CMDS)

 mkdir -p $(TARGET_DIR)/lib/firmware
 cp -rf $(@D)/amadec/firmware/*.bin $(TARGET_DIR)/lib/firmware
 cp -f $(STAGING_DIR)/usr/lib/libamadec.so $(TARGET_DIR)/usr/lib/

 cp -f $(STAGING_DIR)/usr/lib/libamcodec.so.* $(TARGET_DIR)/usr/lib/
 cp -f $(STAGING_DIR)/usr/lib/libamplayer.so $(STAGING_DIR)/usr/lib/libamcontroler.so $(TARGET_DIR)/usr/lib/
 $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" INSTALL_DIR="$(TARGET_DIR)/usr/lib" STAGING="$(TARGET_DIR)/usr" -C $(@D)/amplayer install
endef

$(eval $(call GENTARGETS,package/amlogic,libplayer))
