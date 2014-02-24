#############################################################
#
# wipe
#
#############################################################

WIPE_VERSION = 2.3.1
WIPE_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/wipe
WIPE_SOURCE = wipe-$(WIPE_VERSION).tar.bz2
WIPE_AUTORECONF = YES

define WIPE_INSTALL_TARGET_CMDS
	$(INSTALL) -D $(@D)/wipe $(TARGET_DIR)/usr/bin/wipe
	$(INSTALL) -D $(@D)/wipe.1 $(TARGET_DIR)/usr/share/man/man1/wipe.1
endef

define WIPE_UNINSTALL_TARGET_CMDS
	rm -f $(TARGET_DIR)/usr/bin/wipe
	rm -f $(TARGET_DIR)/usr/share/man/man1/wipe.1
endef

$(eval $(call AUTOTARGETS))
