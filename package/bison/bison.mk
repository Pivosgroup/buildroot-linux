################################################################################
#
# bison
#
################################################################################

BISON_VERSION = 3.0
BISON_SOURCE = bison-$(BISON_VERSION).tar.xz
BISON_SITE = $(BR2_GNU_MIRROR)/bison
BISON_LICENSE = GPLv3+
BISON_LICENSE_FILES = COPYING
BISON_CONF_ENV = ac_cv_path_M4=/usr/bin/m4
BISON_DEPENDENCIES = m4
BISON_AUTORECONF = YES

define BISON_DISABLE_EXAMPLES
	echo 'all install:' > $(@D)/examples/Makefile
endef

BISON_POST_CONFIGURE_HOOKS += BISON_DISABLE_EXAMPLES

$(eval $(autotools-package))
$(eval $(host-autotools-package))
