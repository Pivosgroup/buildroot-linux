################################################################################
#
# xapp_setxkbmap -- Controls the keyboard layout of a running X server.
#
################################################################################

XAPP_SETXKBMAP_VERSION = 1.1.0
XAPP_SETXKBMAP_SOURCE = setxkbmap-$(XAPP_SETXKBMAP_VERSION).tar.bz2
XAPP_SETXKBMAP_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_SETXKBMAP_DEPENDENCIES = xlib_libX11 xlib_libxkbfile

$(eval $(call AUTOTARGETS))
