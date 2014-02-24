#############################################################
#
# minicom
#
#############################################################
MINICOM_VERSION = 2.6.1
MINICOM_SOURCE = minicom-$(MINICOM_VERSION).tar.gz
MINICOM_SITE = http://alioth.debian.org/frs/download.php/3700/

# pkg-config is only used to check for liblockdev, which we don't have
# in BR, so instead of adding host-pkg-config as a dependency, simply
# make sure the host version isn't used so we don't end up with problems
# if people have liblockdev1-dev installed
MINICOM_CONF_ENV = PKG_CONFIG=/bin/false

MINICOM_DEPENDENCIES = ncurses $(if $(BR2_ENABLE_LOCALE),,libiconv)

$(eval $(call AUTOTARGETS))
