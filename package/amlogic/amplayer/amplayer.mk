#############################################################
#
# amplayer
#
#############################################################


AMPLAYER_VERSION:=0.9.9
AMPLAYER_SOURCE=amplayer-$(AMPLAYER_VERSION).tar.gz
AMPLAYER_SITE=./package/amlogic/libplayer/src/amplayer
AMPLAYER_SITE_METHOD=cp
AMPLAYER_INSTALL_STAGING=YES
AMPLAYER_DEPENDENCIES=amffmpeg amcodec

define AMPLAYER_BUILD_CMDS
	$(MAKE)  LD="$(TARGET_LD)" CC="$(TARGET_CC)" SRC=$(CURDIR)/$(AMPLAYER_SITE) -C $(@D)
endef

define AMPLAYER_INSTALL_STAGING_CMDS
	$(MAKE) CC="$(TARGET_CC)" INSTALL_DIR=$(STAGING_DIR)/usr/lib STAGING=$(STAGING_DIR)/usr -C $(@D) install
endef

define AMPLAYER_INSTALL_TARGET_CMDS
	$(MAKE) CC="$(TARGET_CC)" INSTALL_DIR=$(TARGET_DIR)/usr/lib STAGING=$(TARGET_DIR)/usr -C $(@D) install
endef

$(eval $(call GENTARGETS,package/amlogic,amplayer))
