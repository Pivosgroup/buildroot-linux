#############################################################
#
#librtmp
#
#############################################################
LIBRTMP_VERSION = e0056c51cc1710c9a44d2a2c4e2f344fa9cabcf4
LIBRTMP_SOURCE = rtmpdump-$(LIBRTMP_VERSION).tar.gz
LIBRTMP_SITE_METHOD = git
LIBRTMP_SITE = git://git.ffmpeg.org/rtmpdump
LIBRTMP_INSTALL_STAGING = YES

LIBRTMP_DEPENDENCIES = openssl

define LIBRTMP_BUILD_CMDS
	sed -ie "s|prefix=/usr/local|prefix=/usr|" $(@D)/librtmp/Makefile
        $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" AR="$(TARGET_AR)" -C $(@D)/librtmp
endef

define LIBRTMP_INSTALL_STAGING_CMDS
	$(MAKE) -C $(@D)/librtmp install DESTDIR=$(STAGING_DIR)
endef

define LIBRTMP_INSTALL_TARGET_CMDS
	install -m 644 $(@D)/librtmp/librtmp.so.0 $(TARGET_DIR)/usr/lib
endef

$(eval $(call GENTARGETS,package/thirdparty,librtmp))
