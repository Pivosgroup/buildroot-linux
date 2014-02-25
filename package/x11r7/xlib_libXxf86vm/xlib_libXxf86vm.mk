################################################################################
#
# xlib_libXxf86vm -- X.Org Xxf86vm library
#
################################################################################

XLIB_LIBXXF86VM_VERSION = 1.1.1
XLIB_LIBXXF86VM_SOURCE = libXxf86vm-$(XLIB_LIBXXF86VM_VERSION).tar.bz2
XLIB_LIBXXF86VM_SITE = http://xorg.freedesktop.org/releases/individual/lib
XLIB_LIBXXF86VM_INSTALL_STAGING = YES
XLIB_LIBXXF86VM_DEPENDENCIES = xlib_libX11 xlib_libXext xproto_xf86vidmodeproto xproto_xproto
XLIB_LIBXXF86VM_CONF_OPT = --disable-malloc0returnsnull

$(eval $(autotools-package))
