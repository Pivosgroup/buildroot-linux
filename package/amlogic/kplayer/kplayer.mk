#############################################################
#
# kplayer
#
#############################################################
KPLAYER_VERSION:=0.9.9
KPLAYER_SOURCE=kplayer-$(KPLAYER_VERSION).tar.gz
KPLAYER_SITE=./package/amlogic/kplayer/src
KPLAYER_SITE_METHOD=cp
KPLAYER_DEPENDENCIES=amffmpeg amcodec amadec amplayer

define KPLAYER_BUILD_CMDS
        make CC="$(TARGET_CC)" -C $(@D)
endef

define KPLAYER_INSTALL_TARGET_CMDS
        install -m 755 $(@D)/kplayer $(TARGET_DIR)/usr/bin
endef

$(eval $(call GENTARGETS,package/amlogic,kplayer))
