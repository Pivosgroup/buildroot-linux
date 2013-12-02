#############################################################
#
# kismet
#
#############################################################

KISMET_VERSION = 2011-03-R2
KISMET_SITE = http://www.kismetwireless.net/code
KISMET_DEPENDENCIES = libpcap ncurses

ifeq ($(BR2_PACKAGE_LIBNL),y)
	KISMET_DEPENDENCIES += libnl
endif
ifeq ($(BR2_PACKAGE_PCRE),y)
	KISMET_DEPENDENCIES += pcre
endif

ifeq ($(BR2_PACKAGE_KISMET_CLIENT),y)
	KISMET_TARGET_BINARIES += kismet_client
endif

ifeq ($(BR2_PACKAGE_KISMET_SERVER),y)
	KISMET_TARGET_BINARIES += kismet_server
	KISMET_TARGET_CONFIGS += kismet.conf
endif

ifeq ($(BR2_PACKAGE_KISMET_DRONE),y)
	KISMET_TARGET_BINARIES += kismet_drone
	KISMET_TARGET_CONFIGS += kismet_drone.conf
endif

ifdef KISMET_TARGET_BINARIES
define KISMET_INSTALL_TARGET_BINARIES
	$(INSTALL) -m 755 $(addprefix $(KISMET_DIR)/, $(KISMET_TARGET_BINARIES)) $(TARGET_DIR)/usr/bin
endef
endif

ifdef KISMET_TARGET_CONFIGS
define KISMET_INSTALL_TARGET_CONFIGS
	$(INSTALL) -m 644 $(addprefix $(KISMET_DIR)/conf/, $(KISMET_TARGET_CONFIGS)) $(TARGET_DIR)/etc
endef
endif

define KISMET_INSTALL_TARGET_CMDS
	$(KISMET_INSTALL_TARGET_BINARIES)
	$(KISMET_INSTALL_TARGET_CONFIGS)
endef

ifdef KISMET_TARGET_BINARIES
define KISMET_UNINSTALL_TARGET_BINARIES
	rm -f $(addprefix $(TARGET_DIR)/usr/bin/, $(KISMET_TARGET_BINARIES))
endef
endif

ifdef KISMET_TARGET_CONFIGS
define KISMET_UNINSTALL_TARGET_CONFIGS
	rm -f $(addprefix $(TARGET_DIR)/etc/, $(KISMET_TARGET_CONFIGS))
endef
endif

define KISMET_UNINSTALL_TARGET_CMDS
	$(KISMET_UNINSTALL_TARGET_BINARIES)
	$(KISMET_UNINSTALL_TARGET_CONFIGS)
endef

$(eval $(call AUTOTARGETS))
