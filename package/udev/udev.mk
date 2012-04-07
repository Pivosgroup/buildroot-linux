#############################################################
#
# udev
#
#############################################################
UDEV_VERSION = 173
UDEV_SOURCE = udev-$(UDEV_VERSION).tar.bz2
UDEV_SITE = http://linuxfromscratch.pl/pub/LFS/lfs-packages/7.0
UDEV_INSTALL_STAGING = YES

UDEV_CONF_OPT =			\
	--sbindir=/sbin		\
	--with-rootlibdir=/lib	\
	--libexecdir=/lib/udev	\
	--disable-introspection \
  --disable-gtk-doc-html

UDEV_DEPENDENCIES = host-gperf host-pkg-config

define UDEV_REMOVE_MTD_PROBE_RULE
    rm -f $(TARGET_DIR)/lib/udev/rules.d/75-probe_mtd.rules
    rm -f $(STAGING_DIR)/lib/udev/rules.d/75-probe_mtd.rules
endef

ifneq ($(BR2_PACKAGE_UDEV_MTD_PROBE),y)
UDEV_CONF_OPT += --disable-mtd_probe
UDEV_POST_INSTALL_TARGET_HOOKS += UDEV_REMOVE_MTD_PROBE_RULE
endif

ifeq ($(BR2_PACKAGE_UDEV_ALL_EXTRAS),y)
UDEV_DEPENDENCIES += libusb libusb-compat usbutils hwdata libglib2
UDEV_CONF_OPT +=							\
	--with-pci-ids-path=/usr/share/hwdata/pci.ids	\
	--with-usb-ids-path=/usr/share/hwdata/usb.ids	\

else
UDEV_CONF_OPT +=		\
	--disable-hwdb		\
	--disable-gudev
endif

define UDEV_INSTALL_INITSCRIPT
	$(INSTALL) -m 0755 package/udev/S10udev $(TARGET_DIR)/etc/init.d/S10udev
endef

UDEV_POST_INSTALL_TARGET_HOOKS += UDEV_INSTALL_INITSCRIPT

$(eval $(call AUTOTARGETS,package,udev))
