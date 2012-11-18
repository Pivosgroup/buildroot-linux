################################################################################
#
## boblight
#
#################################################################################

BOBLIGHT_VERSION = r458
BOBLIGHT_SITE_METHOD = svn
BOBLIGHT_SITE = http://boblight.googlecode.com/svn/trunk/
BOBLIGHT_INSTALL_STAGING = YES
BOBLIGHT_INSTALL_TARGET = YES
BOBLIGHT_AUTORECONF = YES
BOBLIGHT_CONF_OPT += --without-portaudio --without-opengl --without-x11

BOBLIGHT_DEPENDENCIES += linux26

ifeq ($(findstring yy,$(BR2_PACKAGE_BOBLIGHT_LIBUSB)$(BR2_PACKAGE_LIBUSB)),yy)
  BOBLIGHT_DEPENDENCIES += libusb
else
  BOBLIGHT_CONF_OPT += --without-libusb
endif

$(eval $(call AUTOTARGETS,package/thirdparty,boblight))
