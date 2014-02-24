#############################################################
#
# empty
#
#############################################################
EMPTY_VERSION = 0.6.18b
EMPTY_SOURCE = empty-$(EMPTY_VERSION).tgz
EMPTY_SITE = http://downloads.sourceforge.net/project/empty/empty/empty-$(EMPTY_VERSION)

define EMPTY_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all
endef

define EMPTY_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/empty $(TARGET_DIR)/usr/bin/empty
endef

$(eval $(call GENTARGETS))
