#############################################################
#
# libdnet
#
#############################################################

LIBDNET_VERSION = 1.11
LIBDNET_SOURCE = libdnet-$(LIBDNET_VERSION).tar.gz
LIBDNET_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/libdnet/
LIBDNET_INSTALL_STAGING = YES
LIBDNET_AUTORECONF = YES
LIBDNET_CONF_OPT = \
	--with-gnu-ld \
	--with-check=no

ifneq ($(BR2_PACKAGE_LIBDNET_PYTHON),)
LIBDNET_DEPENDENCIES = python
LIBDNET_CONF_OPT += --with-python
LIBDNET_MAKE_OPT = PYINCDIR=$(STAGING_DIR)/usr/include/python$(PYTHON_VERSION_MAJOR) PYLIBDIR=$(STAGING_DIR)/usr/lib
LIBDNET_INSTALL_TARGET_OPT = $(LIBDNET_MAKE_OPT) DESTDIR=$(TARGET_DIR) INSTALL_STRIP_FLAG=-s install-exec
LIBDNET_INSTALL_STAGING_OPT = $(LIBDNET_MAKE_OPT) DESTDIR=$(STAGING_DIR) install
endif

# Needed for autoreconf to work properly
define LIBDNET_FIXUP_ACINCLUDE_M4
	ln -sf config/acinclude.m4 $(@D)
endef

LIBDNET_POST_EXTRACT_HOOKS += LIBDNET_FIXUP_ACINCLUDE_M4

define LIBDNET_REMOVE_CONFIG_SCRIPT
	$(RM) -f $(TARGET_DIR)/usr/bin/dnet-config
endef

ifneq ($(BR2_HAVE_DEVFILES),y)
LIBDNET_POST_INSTALL_TARGET_HOOKS += LIBDNET_REMOVE_CONFIG_SCRIPT
endif

$(eval $(call AUTOTARGETS))
