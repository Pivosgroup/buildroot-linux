#############################################################
#
# syslinux to make target msdos/iso9660 filesystems bootable
#
#############################################################

SYSLINUX_VERSION = 4.04
SYSLINUX_SOURCE  = syslinux-$(SYSLINUX_VERSION).tar.bz2
SYSLINUX_SITE    = $(BR2_KERNEL_MIRROR)/linux/utils/boot/syslinux/4.xx/

SYSLINUX_INSTALL_TARGET = NO
SYSLINUX_INSTALL_IMAGES = YES

SYSLINUX_DEPENDENCIES = host-nasm

define SYSLINUX_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(HOSTCC)" AR="$(HOSTAR)" -C $(@D)
endef

SYSLINUX_IMAGES-$(BR2_TARGET_SYSLINUX_ISOLINUX) += isolinux.bin
SYSLINUX_IMAGES-$(BR2_TARGET_SYSLINUX_PXELINUX) += pxelinux.bin

define SYSLINUX_INSTALL_IMAGES_CMDS
	for i in $(SYSLINUX_IMAGES-y); do \
		$(INSTALL) -D -m 0755 $(@D)/core/$$i $(BINARIES_DIR)/$$i; \
	done
endef


define HOST_SYSLINUX_BUILD_CMDS
	$(HOST_MAKE_ENV) $(MAKE) -C $(@D)
endef

define HOST_SYSLINUX_INSTALL_CMDS
	$(HOST_MAKE_ENV) $(MAKE) -C $(@D) INSTALLROOT=$(HOST_DIR) install
endef

define HOST_SYSLINUX_CLEAN_CMDS
	$(HOST_MAKE_ENV) $(MAKE) -C $(@D) clean
endef

$(eval $(call GENTARGETS))
$(eval $(call GENTARGETS,host))
