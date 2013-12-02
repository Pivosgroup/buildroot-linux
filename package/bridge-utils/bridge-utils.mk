#############################################################
#
# bridge-utils
#
#############################################################

BRIDGE_UTILS_VERSION = 1.5
BRIDGE_UTILS_SOURCE = bridge-utils-$(BRIDGE_UTILS_VERSION).tar.gz
BRIDGE_UTILS_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/bridge
BRIDGE_UTILS_AUTORECONF = YES
BRIDGE_UTILS_CONF_OPT = --with-linux-headers=$(LINUX_HEADERS_DIR)

define BRIDGE_UTILS_UNINSTALL_TARGET_CMDS
	rm -f $(addprefix $(TARGET_DIR)/usr/,lib/libbridge.a \
		include/libbridge.h share/man/man8/brctl.8 sbin/brctl)
endef

$(eval $(call AUTOTARGETS))
