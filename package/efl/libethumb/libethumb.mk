################################################################################
#
# libethumb
#
################################################################################

LIBETHUMB_VERSION = 0.1.1.65643
LIBETHUMB_SOURCE = ethumb-$(LIBETHUMB_VERSION).tar.bz2
LIBETHUMB_SITE = http://download.enlightenment.org/snapshots/2011-11-28
LIBETHUMB_INSTALL_STAGING = YES

LIBETHUMB_DEPENDENCIES = libeina libevas libecore libedje host-libedje

LIBETHUMB_CONF_OPT = --with-edje-cc=$(HOST_DIR)/usr/bin/edje_cc

ifeq ($(BR2_PACKAGE_LIBEXIF),y)
LIBETHUMB_DEPENDENCIES += libexif
endif

ifeq ($(BR2_PACKAGE_LIBEDBUS),y)
LIBETHUMB_DEPENDENCIES += libedbus
endif

$(eval $(call AUTOTARGETS))
