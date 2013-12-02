################################################################################
#
# xapp_xmore -- plain text display program for the X Window System
#
################################################################################

XAPP_XMORE_VERSION = 1.0.1
XAPP_XMORE_SOURCE = xmore-$(XAPP_XMORE_VERSION).tar.bz2
XAPP_XMORE_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XMORE_DEPENDENCIES = xlib_libXprintUtil xlib_libXaw

$(eval $(call AUTOTARGETS))
