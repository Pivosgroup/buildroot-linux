################################################################################
#
# xlib_libXau -- X.Org Xau library
#
################################################################################

XLIB_LIBXAU_VERSION = 1.0.6
XLIB_LIBXAU_SOURCE = libXau-$(XLIB_LIBXAU_VERSION).tar.bz2
XLIB_LIBXAU_SITE = http://xorg.freedesktop.org/releases/individual/lib
XLIB_LIBXAU_INSTALL_STAGING = YES
XLIB_LIBXAU_DEPENDENCIES = xutil_util-macros xproto_xproto

$(eval $(autotools-package))
$(eval $(host-autotools-package))
