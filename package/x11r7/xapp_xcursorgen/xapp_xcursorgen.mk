################################################################################
#
# xapp_xcursorgen -- create an X cursor file from a collection of PNG images
#
################################################################################

XAPP_XCURSORGEN_VERSION = 1.0.4
XAPP_XCURSORGEN_SOURCE = xcursorgen-$(XAPP_XCURSORGEN_VERSION).tar.bz2
XAPP_XCURSORGEN_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XCURSORGEN_DEPENDENCIES = libpng xlib_libX11 xlib_libXcursor

HOST_XAPP_XCURSORGEN_DEPENDENCIES = host-libpng host-xlib_libX11 host-xlib_libXcursor

$(eval $(call AUTOTARGETS))
$(eval $(call AUTOTARGETS,host))
