#############################################################
#
# libamplayer
#
#############################################################
LIBAMPLAYERM1_VERSION:=e7ba30082fcf17874d1091a455ddaf9f4070a4fa
LIBAMPLAYERM1_SOURCE=libamplayerm1-$(LIBAMPLAYERM1_VERSION).tar.gz
LIBAMPLAYERM1_SITE=$(TOPDIR)/package/amlogic/libamplayerm1/src
LIBAMPLAYERM1_INSTALL_STAGING=YES
LIBAMPLAYERM1_INSTALL_TARGET=YES
LIBAMPLAYERM1_SITE_METHOD=cp

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
endef

define LIBAMPLAYERM1_INSTALL_STAGING_CMDS
	mkdir -p $(STAGING_DIR)/usr/include
	install -m 644 $(@D)/usr/include/*.h $(STAGING_DIR)/usr/include
	mkdir -p $(STAGING_DIR)/usr/include/amlplayer
	install -m 644 $(@D)/usr/include/amlplayer/*.h $(STAGING_DIR)/usr/include/amlplayer
	mkdir -p $(STAGING_DIR)/usr/include/amlplayer/amports
	install -m 644 $(@D)/usr/include/amlplayer/amports/*.h $(STAGING_DIR)/usr/include/amlplayer/amports
	mkdir -p $(STAGING_DIR)/usr/include/amlplayer/ppmgr
	install -m 644 $(@D)/usr/include/amlplayer/ppmgr/*.h $(STAGING_DIR)/usr/include/amlplayer/ppmgr
	mkdir -p $(STAGING_DIR)/usr/lib
	install -m 755 $(@D)/usr/lib/*.so* $(STAGING_DIR)/usr/lib
	ln -s $(STAGING_DIR)/usr/lib/libamcodec.so.0.0 $(STAGING_DIR)/usr/lib/libamcodec.so

	#temporary, until we sync with mainline xbmc
	cp -rf $(@D)/usr/include/amlplayer/* $(STAGING_DIR)/usr/include
endef

define LIBAMPLAYERM1_INSTALL_TARGET_CMDS
	$(call AMFFMPEG_INSTALL_TARGET_CMDS)

	mkdir -p $(TARGET_DIR)/lib/firmware
	install -m 644 $(@D)/lib/firmware/*.bin $(TARGET_DIR)/lib/firmware
	mkdir -p $(TARGET_DIR)/usr/lib
	install -m 755 $(@D)/usr/lib/*.so* $(TARGET_DIR)/usr/lib
	ln -s $(TARGET_DIR)/usr/lib/libamcodec.so.0.0 $(TARGET_DIR)/usr/lib/libamcodec.so
endef

$(eval $(call GENTARGETS,package/amlogic,libamplayerm1))
