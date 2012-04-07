#############################################################
#
# libssh2
#
#############################################################
LIBSSH2_VERSION = 1.2.8
LIBSSH2_SITE = http://www.libssh2.org/download
LIBSSH2_SOURCE = libssh2-$(LIBSSH2_VERSION).tar.gz
LIBSSH2_INSTALL_STAGING = YES
LIBSSH2_INSTALL_TARGET = YES

ifeq ($(BR2_PACKAGE_OPENSSL),y)
LIBSSH2_CONF_OPT += --with-openssl --with-libssl-prefix=$(STAGING_DIR)/usr
LIBSSH2_DEPENDENCIES += openssl
endif

ifeq ($(BR2_PACKAGE_LIBGCRYPT),y)
LIBSSH2_CONF_OPT = --with-libgcrypt --with-libgcrypt-prefix=$(STAGING_DIR)/usr
LIBSSH2_DEPENDENCIES += libgcrypt
endif

ifeq ($(BR2_PACKAGE_LIBGCRYPT)$(BR2_PACKAGE_OPENSSL),n)
LIBSSH2_CONF_OPT = --without-openssl --without-libgcrypt
endif

$(eval $(call AUTOTARGETS,package/thirdparty,libssh2))
