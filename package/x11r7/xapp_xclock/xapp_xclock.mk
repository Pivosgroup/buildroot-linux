################################################################################
#
# xapp_xclock -- analog / digital clock for X
#
################################################################################

XAPP_XCLOCK_VERSION = 1.0.6
XAPP_XCLOCK_SOURCE = xclock-$(XAPP_XCLOCK_VERSION).tar.bz2
XAPP_XCLOCK_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XCLOCK_DEPENDENCIES = xlib_libX11 xlib_libXaw xlib_libXft xlib_libXrender xlib_libxkbfile

$(eval $(autotools-package))
