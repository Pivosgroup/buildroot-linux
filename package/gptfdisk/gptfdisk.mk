################################################################################
#
# gptfdisk
#
################################################################################

GPTFDISK_VERSION = 0.8.6
GPTFDISK_SITE = http://downloads.sourceforge.net/sourceforge/gptfdisk

GPTFDISK_TARGETS_$(BR2_PACKAGE_GPTFDISK_GDISK) += gdisk
GPTFDISK_TARGETS_$(BR2_PACKAGE_GPTFDISK_SGDISK) += sgdisk
GPTFDISK_TARGETS_$(BR2_PACKAGE_GPTFDISK_CGDISK) += cgdisk

GPTFDISK_DEPENDENCIES += util-linux
ifeq ($(BR2_PACKAGE_GPTFDISK_SGDISK),y)
    GPTFDISK_DEPENDENCIES += popt
endif
ifeq ($(BR2_PACKAGE_GPTFDISK_CGDISK),y)
    GPTFDISK_DEPENDENCIES += ncurses
endif

ifeq ($(BR2_PACKAGE_ICU),y)
    GPTFDISK_DEPENDENCIES += icu
    GPTFDISK_MAKE_OPTS += USE_UTF16=y
endif

define GPTFDISK_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) \
		$(GPTFDISK_MAKE_OPTS) $(GPTFDISK_TARGETS_y)
endef

define GPTFDISK_INSTALL_TARGET_CMDS
	for i in $(GPTFDISK_TARGETS_y); do \
	    $(INSTALL) -D -m 0755 $(@D)/$$i $(TARGET_DIR)/usr/sbin/$$i; \
	done
endef

define GPTFDISK_UNINSTALL_TARGET_CMDS
	rm -f $(addprefix $(TARGET_DIR)/usr/sbin/,$(GPTFDISK_TARGETS_y))
endef

$(eval $(generic-package))
