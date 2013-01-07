#############################################################
#
# tar + squashfs to archive target filesystem
#
#############################################################

ROOTFS_TAR_AML_DEPENDENCIES = host-squashfs

ifeq ($(BR2_TARGET_ROOTFS_SQUASHFS_TAR_AML_LZO),y)
TAR_AML_SQUASHFS_ARGS += -comp lzo
else
ifeq ($(BR2_TARGET_ROOTFS_SQUASHFS_TAR_AML_LZMA),y)
TAR_AML_SQUASHFS_ARGS += -comp lzma
else
TAR_AML_SQUASHFS_ARGS += -comp gzip
endif
endif

TAR_AML_CLEANUP = true
ifneq ($(BR2_TARGET_ROOTFS_RECOVERY_AML),y)
TAR_AML_CLEANUP = rm -f $(TARGET_DIR)/usr.sqsh
endif

define ROOTFS_TAR_AML_CMD
 ln -sf ../usr/sbin/reboot $(TARGET_DIR)/sbin/reboot && \
 $(HOST_DIR)/usr/bin/mksquashfs $(TARGET_DIR)/usr $(TARGET_DIR)/usr.sqsh $(TAR_AML_SQUASHFS_ARGS) -noappend -all-root && \
 tar --anchored --exclude=\"./usr/*\" -cf $(BINARIES_DIR)/rootfs.tar -C $(TARGET_DIR) . && $(TAR_AML_CLEANUP)
endef



$(eval $(call ROOTFS_TARGET,tar_aml))
