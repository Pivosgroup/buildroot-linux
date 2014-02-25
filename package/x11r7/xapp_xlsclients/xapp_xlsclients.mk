################################################################################
#
# xapp_xlsclients -- X.Org xlsclients application
#
################################################################################

XAPP_XLSCLIENTS_VERSION = 1.1.1
XAPP_XLSCLIENTS_SOURCE = xlsclients-$(XAPP_XLSCLIENTS_VERSION).tar.bz2
XAPP_XLSCLIENTS_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XLSCLIENTS_DEPENDENCIES = xlib_libX11 xlib_libXmu libxcb xcb-util

$(eval $(autotools-package))
