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

BOBLIGHT_CONF_OPT+= --without-portaudio --without-opengl --without-x11

define BOBLIGHT_BOOTSTRAP
  cd $(BOBLIGHT_DIR) && autoreconf -vif
endef


BOBLIGHT_PRE_CONFIGURE_HOOKS += BOBLIGHT_BOOTSTRAP

$(eval $(call AUTOTARGETS,package/thirdparty,boblight))
