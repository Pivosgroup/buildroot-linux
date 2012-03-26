#############################################################
#
# libshairport
#
#############################################################
LIBSHAIRPORT_VERSION = 1.2.0.20310_lib
LIBSHAIRPORT_SITE = http://mirrors.xbmc.org/build-deps/darwin-libs
LIBSHAIRPORT_SOURCE = libshairport-$(LIBSHAIRPORT_VERSION).tar.gz
LIBSHAIRPORT_INSTALL_STAGING = YES
LIBSHAIRPORT_INSTALL_TARGET = YES
LIBSHAIRPORT_AUTORECONF = YES
LIBSHAIRPORT_DEPENDENCIES += openssl

$(eval $(call AUTOTARGETS,package/thirdparty,libshairport))
