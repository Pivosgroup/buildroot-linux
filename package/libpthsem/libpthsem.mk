################################################################################
#
# libpthsem
#
################################################################################

LIBPTHSEM_VERSION = 2.0.8
LIBPTHSEM_SOURCE = pthsem_$(LIBPTHSEM_VERSION).tar.gz
LIBPTHSEM_SITE = http://www.auto.tuwien.ac.at/~mkoegler/pth/
LIBPTHSEM_LICENSE = LGPLv2.1+
LIBPTHSEM_LICENSE_FILES = COPYING
LIBPTHSEM_AUTORECONF = YES
LIBPTHSEM_INSTALL_STAGING = YES
LIBPTHSEM_DEPENDENCIES = argp-standalone
LIBPTHSEM_CONFIG_SCRIPTS = pthsem-config

$(eval $(autotools-package))
