################################################################################
#
# xlib_libXScrnSaver -- X.Org XScrnSaver library
#
################################################################################

XLIB_LIBXSCRNSAVER_VERSION = 1.2.1
XLIB_LIBXSCRNSAVER_SOURCE = libXScrnSaver-$(XLIB_LIBXSCRNSAVER_VERSION).tar.bz2
XLIB_LIBXSCRNSAVER_SITE = http://xorg.freedesktop.org/releases/individual/lib
XLIB_LIBXSCRNSAVER_INSTALL_STAGING = YES
XLIB_LIBXSCRNSAVER_DEPENDENCIES = xlib_libX11 xlib_libXext xproto_scrnsaverproto
XLIB_LIBXSCRNSAVER_CONF_OPT = --disable-malloc0returnsnull

$(eval $(autotools-package))
