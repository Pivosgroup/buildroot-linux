################################################################################
#
# sunxi-cedarx
#
################################################################################

SUNXI_CEDARX_VERSION = 74923e55fc3ef512d4cd2462da58ae5331611f37
SUNXI_CEDARX_SITE = http://github.com/linux-sunxi/cedarx-libs/tarball/$(SUNXI_CEDARX_VERSION)

SUNXI_CEDARX_INSTALL_STAGING = YES

SUNXI_CEDARX_CONFIGURE_OPTS = \
	CROSS_COMPILE=$(TARGET_CROSS)

ifeq ($(BR2_ARM_EABIHF),y)
SUNXI_CEDARX_BIN_DIR = $(@D)/libcedarv/linux-armhf
# libavheap.so is only available on EABIHF
define SUNXI_CEDARX_BUILD_AVHEAP
	$(TARGET_CC) $(TARGET_CFLAGS) \
		-c $(SUNXI_CEDARX_BIN_DIR)/adapter/avheap/avheap.c \
		-o $(SUNXI_CEDARX_BIN_DIR)/adapter/avheap/avheap.o \
		-I $(SUNXI_CEDARX_BIN_DIR)/adapter/avheap \
		-I $(SUNXI_CEDARX_BIN_DIR)/
	$(TARGET_CC) -shared -L./ -Wl,-soname,libavheap.so \
		-o $(SUNXI_CEDARX_BIN_DIR)/adapter/avheap/libavheap.so \
		$(SUNXI_CEDARX_BIN_DIR)/adapter/avheap/avheap.o
endef
define SUNXI_CEDARX_INSTALL_AVHEAP
	$(INSTALL) -D -m 755 $(SUNXI_CEDARX_BIN_DIR)/adapter/avheap/libavheap.so \
		$(1)/usr/lib/libavheap.so
endef
else
SUNXI_CEDARX_BIN_DIR = $(@D)/libcedarv/linux-armel
endif

define SUNXI_CEDARX_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(SUNXI_CEDARX_BIN_DIR) \
		$(SUNXI_CEDARX_CONFIGURE_OPTS)
	$(SUNXI_CEDARX_BUILD_AVHEAP)
endef

define SUNXI_CEDARX_INSTALL_STAGING_CMDS
	$(INSTALL) -d -m 755 $(STAGING_DIR)/usr/include/libvecore
	$(INSTALL) -m 664 $(SUNXI_CEDARX_BIN_DIR)/libvecore/*.h \
		$(STAGING_DIR)/usr/include/libvecore
	$(INSTALL) -m 644 $(SUNXI_CEDARX_BIN_DIR)/*.h \
		$(STAGING_DIR)/usr/include/
	$(INSTALL) -D -m 755 $(SUNXI_CEDARX_BIN_DIR)/libvecore/libvecore.so \
		$(STAGING_DIR)/usr/lib/libvecore.so
	$(call SUNXI_CEDARX_INSTALL_AVHEAP, $(STAGING_DIR))
endef

define SUNXI_CEDARX_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(SUNXI_CEDARX_BIN_DIR)/libvecore/libvecore.so \
		$(TARGET_DIR)/usr/lib/libvecore.so
	$(call SUNXI_CEDARX_INSTALL_AVHEAP, $(TARGET_DIR))
endef

$(eval $(generic-package))
