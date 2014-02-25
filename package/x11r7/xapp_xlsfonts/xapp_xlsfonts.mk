################################################################################
#
# xapp_xlsfonts -- X.Org xlsfonts application
#
################################################################################

XAPP_XLSFONTS_VERSION = 1.0.1
XAPP_XLSFONTS_SOURCE = xlsfonts-$(XAPP_XLSFONTS_VERSION).tar.bz2
XAPP_XLSFONTS_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XLSFONTS_DEPENDENCIES = xlib_libX11

$(eval $(autotools-package))
