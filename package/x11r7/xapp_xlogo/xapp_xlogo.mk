################################################################################
#
# xapp_xlogo -- X Window System logo
#
################################################################################

XAPP_XLOGO_VERSION = 1.0.1
XAPP_XLOGO_SOURCE = xlogo-$(XAPP_XLOGO_VERSION).tar.bz2
XAPP_XLOGO_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XLOGO_DEPENDENCIES = xlib_libXaw xlib_libXprintUtil xlib_libXrender \
			  xlib_libXft host-pkg-config

$(eval $(call AUTOTARGETS))
