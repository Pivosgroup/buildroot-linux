################################################################################
#
# xlib_libXfontcache -- X.Org Xfontcache library
#
################################################################################

XLIB_LIBXFONTCACHE_VERSION = 1.0.5
XLIB_LIBXFONTCACHE_SOURCE = libXfontcache-$(XLIB_LIBXFONTCACHE_VERSION).tar.bz2
XLIB_LIBXFONTCACHE_SITE = http://xorg.freedesktop.org/releases/individual/lib
XLIB_LIBXFONTCACHE_INSTALL_STAGING = YES
XLIB_LIBXFONTCACHE_DEPENDENCIES = xlib_libX11 xlib_libXext xproto_fontcacheproto
XLIB_LIBXFONTCACHE_CONF_OPT = --disable-malloc0returnsnull

$(eval $(call AUTOTARGETS))
