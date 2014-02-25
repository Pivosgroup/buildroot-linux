##########################################################
#
# CCID
#
# ########################################################
CCID_VERSION = 1.4.8
CCID_SOURCE = ccid-$(CCID_VERSION).tar.bz2
CCID_SITE = http://alioth.debian.org/frs/download.php/3768
CCID_INSTALL_STAGING = YES
CCID_DEPENDENCIES = pcsc-lite host-pkgconf libusb

ifeq ($(BR2_ROOTFS_DEVICE_CREATION_DYNAMIC_UDEV),y)
define CCID_INSTALL_UDEV_RULES
	if test -d $(TARGET_DIR)/etc/udev/rules.d ; then \
		cp $(@D)/src/92_pcscd_ccid.rules $(TARGET_DIR)/etc/udev/rules.d/ ; \
	fi;
endef

CCID_POST_INSTALL_TARGET_HOOKS += CCID_INSTALL_UDEV_RULES
endif

define CCID_REMOVE_UDEV_RULES
	if test -d $(TARGET_DIR)/etc/udev/rules.d ; then \
		rm -f $(TARGET_DIR)/etc/udev/rules.d/92_pcscd_ccid.rules ; \
	fi;
endef
CCID_POST_UNINSTALL_TARGET_HOOKS += CCID_REMOVE_UDEV_RULES

$(eval $(autotools-package))
