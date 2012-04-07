#############################################################
#
# libass
#
#############################################################
LIBASS_VERSION = 0.9.12
LIBASS_SITE = http://libass.googlecode.com/files
LIBASS_SOURCE = libass-$(LIBASS_VERSION).tar.gz
LIBASS_INSTALL_STAGING = YES
LIBASS_INSTALL_TARGET = YES
LIBASS_DEPENDENCIES += libenca

$(eval $(call AUTOTARGETS,package/thirdparty,libass))
