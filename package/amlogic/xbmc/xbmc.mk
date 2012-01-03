XBMC_VERSION = dummy
XBMC_INSTALL_TARGET = YES
XBMC_SOURCE=xbmc-$(XBMC_VERSION).tar.gz
XBMC_SITE=./package/amlogic/xbmc
XBMC_SITE_METHOD=cp
XBMC_DEPENDENCIES = host-lzo host-sdl_image

define XBMC_INSTALL_TARGET_CMDS
  cp -rf package/amlogic/xbmc/etc $(TARGET_DIR)
endef

$(eval $(call GENTARGETS,package/amlogic,xbmc))
