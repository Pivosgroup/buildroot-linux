#############################################################
#
# libamplayer
#
#############################################################
LIBAMPLAYERM3_VERSION:=e690701dbe22a79a2ed55953031713013e5d7d7e
LIBAMPLAYERM3_SOURCE=libamplayerm3-$(LIBAMPLAYERM3_VERSION).tar.gz
LIBAMPLAYERM3_SITE=$(TOPDIR)/package/amlogic/libamplayerm3/src
LIBAMPLAYERM3_SITE_METHOD=cp
LIBAMPLAYERM3_INSTALL_STAGING=YES
LIBAMPLAYERM3_INSTALL_TARGET=YES

ifeq ($(BR2_PACKAGE_LIBAMPLAYERM3),y)
# actually required for amavutils and amffmpeg
LIBAMPLAYERM3_DEPENDENCIES += alsa-lib librtmp pkg-config
AMFFMPEG_DIR = $(BUILD_DIR)/libamplayerm3-$(LIBAMPLAYERM3_VERSION)/amffmpeg
AMAVUTILS_DIR = $(BUILD_DIR)/libamplayerm3-$(LIBAMPLAYERM3_VERSION)/amavutils
AMFFMPEG_EXTRA_LDFLAGS += --extra-ldflags="-lamavutils"
endif

define LIBAMPLAYERM3_BUILD_CMDS
 $(call AMAVUTILS_INSTALL_STAGING_CMDS)
 $(call AMFFMPEG_CONFIGURE_CMDS)
 $(call AMFFMPEG_BUILD_CMDS)
 $(call AMFFMPEG_INSTALL_STAGING_CMDS)
endef

define LIBAMPLAYERM3_INSTALL_STAGING_CMDS
	#find $(@D)/usr/include -type f -exec install -m 644 {} $(STAGING_DIR)/usr/include \;
	mkdir -p $(STAGING_DIR)/usr/include
	install -m 644 $(@D)/usr/include/*.h $(STAGING_DIR)/usr/include
	mkdir -p $(STAGING_DIR)/usr/include/amlplayer
	install -m 644 $(@D)/usr/include/amlplayer/*.h $(STAGING_DIR)/usr/include/amlplayer
	mkdir -p $(STAGING_DIR)/usr/include/amlplayer/amports
	install -m 644 $(@D)/usr/include/amlplayer/amports/*.h $(STAGING_DIR)/usr/include/amlplayer/amports
	mkdir -p $(STAGING_DIR)/usr/include/amlplayer/ppmgr
	install -m 644 $(@D)/usr/include/amlplayer/ppmgr/*.h $(STAGING_DIR)/usr/include/amlplayer/ppmgr
	mkdir -p $(STAGING_DIR)/usr/lib
	install -m 755 $(@D)/usr/lib/libamadec.so $(STAGING_DIR)/usr/lib
	install -m 755 $(@D)/usr/lib/libamcodec.so.0.0 $(STAGING_DIR)/usr/lib
	install -m 755 $(@D)/usr/lib/libamplayer.so $(STAGING_DIR)/usr/lib
	if pushd $(STAGING_DIR)/usr/lib; then \
	  ln -sf libamcodec.so.0.0 libamcodec.so; \
	  popd; \
	else \
	  echo "ERROR: Failed to change directores ($(STAGING_DIR)/usr/lib)"; \
	  exit 1; \
	fi
	#find $(@D)/usr -type f -exec install -m 644 {} $(STAGING_DIR)/usr \;

	#temporary, until we sync with mainline xbmc
	cp -rf $(@D)/usr/include/amlplayer/* $(STAGING_DIR)/usr/include
endef

define LIBAMPLAYERM3_INSTALL_TARGET_CMDS
	$(call AMAVUTILS_INSTALL_TARGET_CMDS)
	$(call AMFFMPEG_INSTALL_TARGET_CMDS)

	mkdir -p $(TARGET_DIR)/lib/firmware
	install -m 644 $(@D)/lib/firmware/*.bin $(TARGET_DIR)/lib/firmware
	mkdir -p $(TARGET_DIR)/usr/lib
	install -m 755 $(@D)/usr/lib/libamadec.so $(TARGET_DIR)/usr/lib
	install -m 755 $(@D)/usr/lib/libamcodec.so.0.0 $(TARGET_DIR)/usr/lib
	install -m 755 $(@D)/usr/lib/libamplayer.so $(TARGET_DIR)/usr/lib
	if pushd $(TARGET_DIR)/usr/lib; then \
	  ln -sf libamcodec.so.0.0 libamcodec.so; \
	  popd; \
	else \
	  echo "ERROR: Failed to change directores ($(TARGET_DIR)/usr/lib)"; \
	  exit 1; \
	fi
#	find $(@D)/lib -type f -exec install -m 644 {} $(TARGET_DIR)/lib \;
#	find $(@D)/usr/lib -type f -exec install -m 644 {} $(TARGET_DIR)/usr/lib \;
endef

$(eval $(call GENTARGETS,package/amlogic,libamplayerm3))
