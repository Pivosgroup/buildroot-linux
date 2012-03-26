#############################################################
#
# libbluray
#
#############################################################
LIBBLURAY_VERSION = 0.2.2
LIBBLURAY_SITE = ftp://ftp.videolan.org/pub/videolan/libbluray/$(LIBBLURAY_VERSION)
LIBBLURAY_SOURCE = libbluray-$(LIBBLURAY_VERSION).tar.bz2
LIBBLURAY_INSTALL_STAGING = YES
LIBBLURAY_INSTALL_TARGET = YES
LIBBLURAY_AUTORECONF = YES

$(eval $(call AUTOTARGETS,package/thirdparty,libbluray))
