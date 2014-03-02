XIOS_SKIN_VERSION = 78e564bc2d8411a328459b9957741cd3e3493074
XIOS_SKIN_SITE_METHOD = git
XIOS_SKIN_SITE = git://github.com/Pivosgroup/skin.pivos.git
XIOS_SKIN_INSTALL_STAGING = YES
XIOS_SKIN_INSTALL_TARGET = YES
XIOS_SKIN_DEPENDENCIES = xbmc
TEXTURE_PACKER=$(XBMC_DIR)/tools/TexturePacker/TexturePacker

define XIOS_SKIN_BUILD_CMDS
	$(TEXTURE_PACKER) -use_none -i $(@D)/media -o $(@D)/media/Textures.xbt
endef

define XIOS_SKIN_INSTALL_STAGING_CMDS
        mkdir -p $(STAGING_DIR)/usr/share/xbmc/addons/skin.xios
        cp -rf $(@D)/* $(STAGING_DIR)/usr/share/xbmc/addons/skin.xios/
endef

define XIOS_SKIN_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/usr/share/xbmc/addons/skin.xios/media
	cd $(@D); cp -rf `ls -I media -1` $(TARGET_DIR)/usr/share/xbmc/addons/skin.xios
	cp -f $(@D)/media/Textures.xbt $(TARGET_DIR)/usr/share/xbmc/addons/skin.xios/media
endef

$(eval $(generic-package))
