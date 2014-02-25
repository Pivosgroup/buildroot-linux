################################################################################
#
# xapp_xgamma -- Alter a monitor's gamma correction through the X server
#
################################################################################

XAPP_XGAMMA_VERSION = 1.0.4
XAPP_XGAMMA_SOURCE = xgamma-$(XAPP_XGAMMA_VERSION).tar.bz2
XAPP_XGAMMA_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XGAMMA_DEPENDENCIES = xlib_libXxf86vm

$(eval $(autotools-package))
