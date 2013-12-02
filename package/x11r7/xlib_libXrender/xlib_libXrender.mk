################################################################################
#
# xlib_libXrender -- X.Org Xrender library
#
################################################################################

XLIB_LIBXRENDER_VERSION = 0.9.5
XLIB_LIBXRENDER_SOURCE = libXrender-$(XLIB_LIBXRENDER_VERSION).tar.bz2
XLIB_LIBXRENDER_SITE = http://xorg.freedesktop.org/releases/individual/lib
XLIB_LIBXRENDER_INSTALL_STAGING = YES
XLIB_LIBXRENDER_DEPENDENCIES = xlib_libX11 xproto_renderproto xproto_xproto
XLIB_LIBXRENDER_CONF_OPT = --disable-malloc0returnsnull

HOST_XLIB_LIBXRENDER_DEPENDENCIES = host-xlib_libX11 host-xproto_renderproto host-xproto_xproto

$(eval $(call AUTOTARGETS))
$(eval $(call AUTOTARGETS,host))
