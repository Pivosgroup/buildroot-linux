#############################################################
#
# FLUXBOX
#
#############################################################

FLUXBOX_VERSION = 1.3.2
FLUXBOX_SOURCE = fluxbox-$(FLUXBOX_VERSION).tar.bz2
FLUXBOX_SITE = http://downloads.sourceforge.net/project/fluxbox/fluxbox/$(FLUXBOX_VERSION)
FLUXBOX_LICENSE = MIT
FLUXBOX_LICENSE_FILES = COPYING

FLUXBOX_CONF_OPT = --x-includes=$(STAGING_DIR)/usr/include/X11 \
		   --x-libraries=$(STAGING_DIR)/usr/lib

FLUXBOX_DEPENDENCIES = xlib_libX11 $(if $(BR2_PACKAGE_LIBICONV),libiconv)

define FLUXBOX_INSTALL_XSESSION_FILE
	[ -f $(TARGET_DIR)/root/.xsession ] || $(INSTALL) -m 0755 -D \
		package/fluxbox/xsession $(TARGET_DIR)/root/.xsession
endef

FLUXBOX_POST_INSTALL_TARGET_HOOKS += FLUXBOX_INSTALL_XSESSION_FILE

$(eval $(autotools-package))
