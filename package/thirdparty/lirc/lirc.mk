LIRC_VERSION = 0.8.7
LIRC_SOURCE = lirc-$(LIRC_VERSION).tar.gz
LIRC_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/project/lirc/LIRC/$(LIRC_VERSION)
LIRC_INSTALL_STAGING = YES
LIRC_INSTALL_TARGET = YES
LIRC_DEPENDENCIES = linux26
LIRC_MAKE=$(MAKE1)

LIRC_CONF_OPT += --with-kerneldir=$(LINUX26_DIR)
LIRC_CONF_OPT += --with-driver=all
LIRC_CONF_OPT += --with-moduledir="/lib/modules/$(LINUX26_VERSION_PROBED)/misc"

# hack to avoid mknod (requires root). This will be populated automatically.
LIRC_CONF_OPT += ac_cv_path_mknod=$(shell which echo)

# disable X support
LIRC_CONF_OPT += --without-x

ifeq ($(BR2_TOOLCHAIN_EXTERNAL),y)
LIRC_MAKE_ENV += PATH=$(TOOLCHAIN_EXTERNAL_DIR)/bin:$(TARGET_PATH)
LIRC_CONF_ENV += PATH=$(TOOLCHAIN_EXTERNAL_DIR)/bin:$(TARGET_PATH)
else
LIRC_MAKE_ENV += PATH=$(TOOLCHAIN_DIR)/bin:$(TARGET_PATH)
LIRC_CONF_ENV += PATH=$(TOOLCHAIN_DIR)/bin:$(TARGET_PATH)
endif

#work-around for hard-coded depmod
define LIRC_DEPMOD
$(HOST_DIR)/usr/sbin/depmod -b $(TARGET_DIR) -a $(LINUX26_VERSION_PROBED)
endef

define LIRC_REMOVE_BROKEN_DRIVERS
sed -i 's/lirc_wpc8769l//' $(@D)/drivers/Makefile
endef

define LIRC_INSTALL_ETC
  cp -rf package/thirdparty/lirc/etc $(TARGET_DIR)
endef

LIRC_POST_CONFIGURE_HOOKS += LIRC_REMOVE_BROKEN_DRIVERS
LIRC_POST_INSTALL_TARGET_HOOKS += LIRC_DEPMOD
LIRC_POST_INSTALL_TARGET_HOOKS += LIRC_INSTALL_ETC

$(eval $(call AUTOTARGETS,package/thirdparty,lirc))

