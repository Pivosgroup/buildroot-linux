################################################################################
#
# libsysfs
#
################################################################################

LIBSYSFS_VERSION = 2.1.0
LIBSYSFS_SITE = http://downloads.sourceforge.net/project/linux-diag/sysfsutils/$(LIBSYSFS_VERSION)
LIBSYSFS_SOURCE = sysfsutils-$(LIBSYSFS_VERSION).tar.gz
LIBSYSFS_INSTALL_STAGING = YES

$(eval $(autotools-package))
