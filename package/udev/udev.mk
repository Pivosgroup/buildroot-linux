#############################################################
#
# udev
#
#############################################################
UDEV_VERSION = 182
UDEV_SOURCE = udev-$(UDEV_VERSION).tar.bz2
UDEV_SITE = $(BR2_KERNEL_MIRROR)/linux/utils/kernel/hotplug/
UDEV_INSTALL_STAGING = YES

# mq_getattr is in librt
UDEV_CONF_ENV += LIBS=-lrt

UDEV_CONF_OPT =			\
	--sbindir=/sbin		\
	--with-rootlibdir=/lib	\
	--libexecdir=/lib	\
	--with-usb-ids-path=/usr/share/hwdata/usb.ids	\
	--with-pci-ids-path=/usr/share/hwdata/pci.ids	\
	--with-firmware-path=/lib/firmware		\
	--disable-introspection

UDEV_DEPENDENCIES = host-gperf host-pkgconf util-linux kmod

define UDEV_REMOVE_MTD_PROBE_RULE
    rm -f $(TARGET_DIR)/lib/udev/rules.d/75-probe_mtd.rules
    rm -f $(STAGING_DIR)/lib/udev/rules.d/75-probe_mtd.rules
endef

ifneq ($(BR2_PACKAGE_UDEV_MTD_PROBE),y)
UDEV_CONF_OPT += --disable-mtd_probe
UDEV_POST_INSTALL_TARGET_HOOKS += UDEV_REMOVE_MTD_PROBE_RULE
endif

ifeq ($(BR2_PACKAGE_UDEV_RULES_GEN),y)
UDEV_CONF_OPT += --enable-rule_generator
endif

ifeq ($(BR2_PACKAGE_UDEV_ALL_EXTRAS),y)
UDEV_DEPENDENCIES += acl hwdata libglib2
UDEV_CONF_OPT +=		\
	--enable-udev_acl
else
UDEV_CONF_OPT +=		\
	--disable-gudev
endif

ifeq ($(BR2_PACKAGE_SYSTEMD),y)
	UDEV_CONF_OPT += --with-systemdsystemunitdir=/lib/systemd/system/
endif

define UDEV_INSTALL_INITSCRIPT
	$(INSTALL) -m 0755 package/udev/S10udev $(TARGET_DIR)/etc/init.d/S10udev
endef

UDEV_POST_INSTALL_TARGET_HOOKS += UDEV_INSTALL_INITSCRIPT

$(eval $(autotools-package))
