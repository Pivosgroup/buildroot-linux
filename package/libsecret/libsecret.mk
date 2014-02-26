################################################################################
#
# libsecret
#
################################################################################

LIBSECRET_VERSION = 0.15
LIBSECRET_SITE = http://ftp.gnome.org/pub/GNOME/sources/libsecret/$(LIBSECRET_VERSION)
LIBSECRET_SOURCE = libsecret-$(LIBSECRET_VERSION).tar.xz
LIBSECRET_LICENSE = LGPLv2.1+
LIBSECRET_LICENSE_FILES = COPYING
LIBSECRET_INSTALL_STAGING = YES

LIBSECRET_DEPENDENCIES = libglib2 host-intltool
LIBSECRET_CONF_OPT = --disable-manpages --disable-strict --disable-coverage --enable-vala=no

ifeq ($(BR2_PACKAGE_LIBGCRYPT),y)
	LIBSECRET_DEPENDENCIES += libgcrypt
	LIBSECRET_CONF_OPT += --enable-gcrypt \
		--with-libgcrypt-prefix=$(STAGING_DIR)/usr
else
	LIBSECRET_CONF_OPT += --disable-gcrypt
endif

$(eval $(autotools-package))
