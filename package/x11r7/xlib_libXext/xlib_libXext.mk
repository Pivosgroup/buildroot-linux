################################################################################
#
# xlib_libXext -- X.Org Xext library
#
################################################################################

XLIB_LIBXEXT_VERSION = 1.1.1
XLIB_LIBXEXT_SOURCE = libXext-$(XLIB_LIBXEXT_VERSION).tar.bz2
XLIB_LIBXEXT_SITE = http://xorg.freedesktop.org/releases/individual/lib
XLIB_LIBXEXT_INSTALL_STAGING = YES
XLIB_LIBXEXT_DEPENDENCIES = xlib_libX11 xproto_xextproto xproto_xproto
XLIB_LIBXEXT_CONF_OPT = --disable-malloc0returnsnull

$(eval $(call AUTOTARGETS))
