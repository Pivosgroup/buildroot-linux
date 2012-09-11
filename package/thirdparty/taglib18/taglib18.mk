#############################################################
#
# taglib
#
#############################################################
TAGLIB18_VERSION = 1.8
TAGLIB18_SOURCE = taglib-$(TAGLIB18_VERSION).tar.gz
TAGLIB18_SITE = http://mirrors.xbmc.org/build-deps/sources
TAGLIB18_LIBTOOL_PATCH = NO
TAGLIB18_INSTALL_STAGING = YES

define TAGLIB18_REMOVE_DEVFILE
	rm -f $(TARGET_DIR)/usr/bin/taglib-config
endef

ifneq ($(BR2_HAVE_DEVFILES),y)
TAGLIB18_POST_INSTALL_TARGET_HOOKS += TAGLIB18_REMOVE_DEVFILE
endif

$(eval $(call CMAKETARGETS,package/thirdparty,taglib18))
