################################################################################
#
# duma
#
################################################################################

DUMA_VERSION = 2_5_15
DUMA_SOURCE = duma_$(DUMA_VERSION).tar.gz
DUMA_SITE = http://downloads.sourceforge.net/project/duma/duma/2.5.15
DUMA_LICENSE = GPLv2+ LGPLv2.1+
DUMA_LICENSE_FILES = COPYING-GPL COPYING-LGPL

DUMA_INSTALL_STAGING = YES

DUMA_OPTIONS = \
	$(if $(BR2_PACKAGE_DUMA_NO_LEAKDETECTION),-DDUMA_LIB_NO_LEAKDETECTION)

# The dependency of some source files in duma_config.h, which is generated at
# build time, is not specified in the Makefile. Force non-parallel build.
define DUMA_BUILD_CMDS
	$(MAKE1) $(TARGET_CONFIGURE_OPTS)       \
		DUMA_OPTIONS="$(DUMA_OPTIONS)"   \
		$(DUMA_CPP) -C $(@D)
endef

define DUMA_INSTALL_STAGING_CMDS
	$(MAKE) prefix=$(STAGING_DIR)/usr install -C $(@D)
endef

define DUMA_INSTALL_TARGET_CMDS
	$(MAKE) prefix=$(TARGET_DIR)/usr install -C $(@D)
endef

$(eval $(generic-package))
