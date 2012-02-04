#############################################################
#
# libcdio
#
#############################################################
LIBCDIO_VERSION = 0.82
LIBCDIO_SITE = http://ftp.gnu.org/gnu/libcdio/
LIBCDIO_SOURCE = libcdio-$(LIBCDIO_VERSION).tar.gz
LIBCDIO_INSTALL_STAGING = YES
LIBCDIO_INSTALL_TARGET = YES

$(eval $(call AUTOTARGETS,package/thirdparty,libcdio))
