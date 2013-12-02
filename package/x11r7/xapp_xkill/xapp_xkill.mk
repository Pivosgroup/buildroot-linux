################################################################################
#
# xapp_xkill -- kill a client by its X resource
#
################################################################################

XAPP_XKILL_VERSION = 1.0.2
XAPP_XKILL_SOURCE = xkill-$(XAPP_XKILL_VERSION).tar.bz2
XAPP_XKILL_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XKILL_DEPENDENCIES = xlib_libX11 xlib_libXmu

$(eval $(call AUTOTARGETS))
