#############################################################
#
# LIBGPHOTO2
#
#############################################################

LIBGPHOTO2_VERSION = 2.5.0
LIBGPHOTO2_SOURCE = libgphoto2-$(LIBGPHOTO2_VERSION).tar.gz
LIBGPHOTO2_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/gphoto/libgphoto
LIBGPHOTO2_INSTALL_STAGING = YES
LIBGPHOTO2_INSTALL_TARGET = YES
LIBGPHOTO2_CONF_OPT = --enable-static --enable-shared
LIBGPHOTO2_DEPENDENCIES = host-pkg-config libusb-compat libtool
HOST_LIBGPHOTO2_CONF_OPT = --enable-shared --enable-static --without-libusb-1.0 --without-libusb

$(eval $(call AUTOTARGETS,package/thirdparty,libgphoto2))
$(eval $(call AUTOTARGETS,package/thirdparty,libgphoto2,host))
