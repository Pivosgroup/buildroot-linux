################################################################################
#
# xapp_oclock -- round X clock
#
################################################################################

XAPP_OCLOCK_VERSION = 1.0.1
XAPP_OCLOCK_SOURCE = oclock-$(XAPP_OCLOCK_VERSION).tar.bz2
XAPP_OCLOCK_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_OCLOCK_DEPENDENCIES = xlib_libX11 xlib_libXext xlib_libXmu

$(eval $(autotools-package))
