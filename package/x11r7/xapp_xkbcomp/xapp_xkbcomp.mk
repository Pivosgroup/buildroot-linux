################################################################################
#
# xapp_xkbcomp -- compile XKB keyboard description
#
################################################################################

XAPP_XKBCOMP_VERSION = 1.2.0
XAPP_XKBCOMP_SOURCE = xkbcomp-$(XAPP_XKBCOMP_VERSION).tar.bz2
XAPP_XKBCOMP_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XKBCOMP_DEPENDENCIES = xlib_libX11 xlib_libxkbfile
XAPP_XKBCOMP_CONF_ENV = ac_cv_file___xkbparse_c=yes

$(eval $(autotools-package))
$(eval $(host-autotools-package))
