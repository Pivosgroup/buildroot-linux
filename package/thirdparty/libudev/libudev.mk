#############################################################
#
# libudev (c library for accessing udev)
#
#############################################################
LIBUDEV_VERSION = 151
LIBUDEV_SITE = ftp://ftp.lfs-matrix.net/pub/lfs/lfs-packages/6.6/
LIBUDEV_SOURCE = udev-$(LIBUDEV_VERSION).tar.bz2
LIBUDEV_INSTALL_STAGING = YES
LIBUDEV_INSTALL_TARGET = YES
LIBUDEV_CONF_OPT += --disable-gudev --disable-rule_generator --disable-extras
LIBUDEV_CONF_OPT += --disable-introspection --disable-hwdb --disable-keymap --disable-mtd_probe

LIBUDEV_INSTALL_STAGING_OPT  = DESTDIR=$(STAGING_DIR)
LIBUDEV_INSTALL_STAGING_OPT += install-libLTLIBRARIES
LIBUDEV_INSTALL_STAGING_OPT += install-includeHEADERS
LIBUDEV_INSTALL_STAGING_OPT += install-pkgconfigDATA
LIBUDEV_INSTALL_STAGING_OPT += install-sharepkgconfigDATA

LIBUDEV_INSTALL_TARGET_OPT   = DESTDIR=$(TARGET_DIR)
LIBUDEV_INSTALL_TARGET_OPT  += install-libLTLIBRARIES

$(eval $(call AUTOTARGETS,package/thirdparty,libudev))
