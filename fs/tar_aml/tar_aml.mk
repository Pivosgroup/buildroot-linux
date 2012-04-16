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

define ROOTFS_TAR_AML_CMD
 $(HOST_DIR)/usr/bin/mksquashfs $(TARGET_DIR)/usr $(TARGET_DIR)/usr.sqsh $(TAR_AML_SQUASHFS_ARGS) -noappend -all-root && \
 tar --anchored --exclude=\"./usr/*\" -cf $(BINARIES_DIR)/rootfs.tar -C $(TARGET_DIR) . && \
 rm $(TARGET_DIR)/usr.sqsh
endef

$(eval $(call ROOTFS_TARGET,tar_aml))
