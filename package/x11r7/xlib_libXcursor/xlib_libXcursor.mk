################################################################################
#
# xlib_libXcursor -- X.Org Xcursor library
#
################################################################################

XLIB_LIBXCURSOR_VERSION = 1.1.10
XLIB_LIBXCURSOR_SOURCE = libXcursor-$(XLIB_LIBXCURSOR_VERSION).tar.bz2
XLIB_LIBXCURSOR_SITE = http://xorg.freedesktop.org/releases/individual/lib
XLIB_LIBXCURSOR_INSTALL_STAGING = YES
XLIB_LIBXCURSOR_DEPENDENCIES = xlib_libX11 xlib_libXfixes xlib_libXrender xproto_xproto

HOST_XLIB_LIBXCURSOR_DEPENDENCIES = host-xlib_libX11 host-xlib_libXfixes host-xlib_libXrender host-xproto_xproto

$(eval $(call AUTOTARGETS))
$(eval $(call AUTOTARGETS,host))
