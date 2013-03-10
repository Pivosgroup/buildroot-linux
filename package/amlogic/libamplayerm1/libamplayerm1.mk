#############################################################
#
# libamplayer
#
#############################################################
LIBAMPLAYERM1_VERSION:=e7ba30082fcf17874d1091a455ddaf9f4070a4fa
LIBAMPLAYERM1_SITE=git://github.com/Pivosgroup/libamplayer-m1.git
LIBAMPLAYERM1_INSTALL_STAGING=YES
LIBAMPLAYERM1_INSTALL_TARGET=YES
LIBAMPLAYERM1_SITE_METHOD=git

ifeq ($(BR2_PACKAGE_LIBAMPLAYERM1),y)
# actually required for amffmpeg
LIBAMPLAYERM1_DEPENDENCIES += alsa-lib librtmp pkg-config
AMFFMPEG_DIR = $(BUILD_DIR)/libamplayerm1-$(LIBAMPLAYERM1_VERSION)/amffmpeg
AMFFMPEG_EXTRA_INCLUDES += -I$(AMFFMPEG_DIR)/../amavutils/include
endif


define LIBAMPLAYERM1_BUILD_CMDS
 $(call AMFFMPEG_CONFIGURE_CMDS)
 $(call AMFFMPEG_BUILD_CMDS)
 $(call AMFFMPEG_INSTALL_STAGING_CMDS)

 mkdir -p $(STAGING_DIR)/usr/include/amlplayer
 $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" HEADERS_DIR="$(STAGING_DIR)/usr/include/amlplayer" \
  CROSS_PREFIX="$(TARGET_CROSS)" SYSROOT="$(STAGING_DIR)" PREFIX="$(STAGING_DIR)/usr" -C $(@D)/amadec install
 $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" HEADERS_DIR="$(STAGING_DIR)/usr/include/amlplayer" CROSS_PREFIX="$(TARGET_CROSS)" \
  SYSROOT="$(STAGING_DIR)" PREFIX="$(STAGING_DIR)/usr" SRC=$(@D)/amcodec -C $(@D)/amcodec install
 $(MAKE) CROSS="$(TARGET_CROSS)" CC="$(TARGET_CC)" LD="$(TARGET_LD)" PREFIX="$(STAGING_DIR)/usr" \
  SRC="$(@D)/amplayer" -C $(@D)/amplayer
endef

define LIBAMPLAYERM1_INSTALL_STAGING_CMDS
 $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" INSTALL_DIR="$(STAGING_DIR)/usr/lib" \
  STAGING="$(STAGING_DIR)/usr" PREFIX="$(STAGING_DIR)/usr" -C $(@D)/amplayer install

 #temporary, until we sync with mainline xbmc
 cp -rf $(@D)/amcodec/include/* $(STAGING_DIR)/usr/include
endef

define LIBAMPLAYERM1_INSTALL_TARGET_CMDS
 $(call AMFFMPEG_INSTALL_TARGET_CMDS)

 mkdir -p $(TARGET_DIR)/lib/firmware
 cp -rf $(@D)/amadec/firmware/*.bin $(TARGET_DIR)/lib/firmware
 cp -f $(STAGING_DIR)/usr/lib/libamadec.so $(TARGET_DIR)/usr/lib/

 cp -f $(STAGING_DIR)/usr/lib/libamcodec.so.* $(TARGET_DIR)/usr/lib/
 cp -f $(STAGING_DIR)/usr/lib/libamplayer.so $(STAGING_DIR)/usr/lib/libamcontroler.so $(TARGET_DIR)/usr/lib/
 $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" INSTALL_DIR="$(TARGET_DIR)/usr/lib" \
  STAGING="$(TARGET_DIR)/usr" PREFIX="$(STAGING_DIR)/usr" -C $(@D)/amplayer install
endef

$(eval $(call GENTARGETS,package/amlogic,libamplayerm1))
