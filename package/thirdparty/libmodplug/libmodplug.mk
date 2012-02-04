#############################################################
#
# libmodplug
#
#############################################################
LIBMODPLUG_VERSION = 0.8.7
LIBMODPLUG_SITE = http://downloads.sourceforge.net/project/modplug-xmms/libmodplug/0.8.7/
LIBMODPLUG_SOURCE = libmodplug-$(LIBMODPLUG_VERSION).tar.gz
LIBMODPLUG_INSTALL_STAGING = YES
LIBMODPLUG_INSTALL_TARGET = YES

$(eval $(call AUTOTARGETS,package/thirdparty,libmodplug))
