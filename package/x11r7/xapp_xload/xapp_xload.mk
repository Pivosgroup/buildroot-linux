################################################################################
#
# xapp_xload -- system load average display for X
#
################################################################################

XAPP_XLOAD_VERSION = 1.0.2
XAPP_XLOAD_SOURCE = xload-$(XAPP_XLOAD_VERSION).tar.bz2
XAPP_XLOAD_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XLOAD_DEPENDENCIES = xlib_libXaw

$(eval $(call AUTOTARGETS))
