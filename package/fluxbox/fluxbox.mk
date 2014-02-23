#############################################################
#
# FLUXBOX
#
#############################################################

FLUXBOX_VERSION = 1.3.1
FLUXBOX_SOURCE = fluxbox-$(FLUXBOX_VERSION).tar.bz2
FLUXBOX_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/fluxbox/

FLUXBOX_CONF_OPT = --x-includes=$(STAGING_DIR)/usr/include/X11 \
		   --x-libraries=$(STAGING_DIR)/usr/lib

FLUXBOX_DEPENDENCIES = xlib_libX11 $(if $(BR2_PACKAGE_LIBICONV),libiconv)

$(eval $(call AUTOTARGETS))
