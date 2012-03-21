#############################################################
#
# amremote_config
#
#############################################################
AMREMOTE_CONFIG_VERSION:=0.9.9
AMREMOTE_CONFIG_SOURCE=amremote_config-$(AMREMOTE_CONFIG_VERSION).tar.gz
AMREMOTE_CONFIG_SITE=./package/amlogic/amremote_config/src
AMREMOTE_CONFIG_SITE_METHOD=cp

define AMREMOTE_CONFIG_BUILD_CMDS
        $(MAKE) CC="$(TARGET_CC)" -C $(@D)
endef

define AMREMOTE_CONFIG_INSTALL_TARGET_CMDS
        install -m 755 $(@D)/keytest $(TARGET_DIR)/usr/bin
        install -m 755 $(@D)/amremote_config $(TARGET_DIR)/usr/bin
endef

$(eval $(call GENTARGETS,package/amlogic,amremote_config))
