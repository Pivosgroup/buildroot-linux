#############################################################
#
# tar + squashfs to archive target filesystem
#
#############################################################

ROOTFS_RECOVERY_AML_DEPENDENCIES = linux26 rootfs-tar_aml

RECOVERY_AML_ARGS = -b $(BR2_TARGET_ROOTFS_RECOVERY_AML_BOARDNAME)
ifeq ($(BR2_TARGET_ROOTFS_RECOVERY_AML_WIPE_USERDATA),y)
  RECOVERY_AML_ARGS += -w
endif
ifeq ($(BR2_TARGET_ROOTFS_RECOVERY_AML_WIPE_USERDATA_CONDITIONAL),y)
  RECOVERY_AML_ARGS += -c
endif

define ROOTFS_RECOVERY_AML_CMD
 mkdir -p $(BINARIES_DIR)/aml_recovery/system && \
 tar -C $(BINARIES_DIR)/aml_recovery/system -xf $(BINARIES_DIR)/rootfs.tar && \
 mkdir -p $(BINARIES_DIR)/aml_recovery/META-INF/com/google/android/ && \
 PYTHONDONTWRITEBYTECODE=1 fs/recovery_aml/android_scriptgen $(RECOVERY_AML_ARGS) -i -p $(BINARIES_DIR)/aml_recovery/system -o \
   $(BINARIES_DIR)/aml_recovery/META-INF/com/google/android/updater-script && \
 cp -f fs/recovery_aml/update-binary $(BINARIES_DIR)/aml_recovery/META-INF/com/google/android/ && \
 cp -f fs/recovery_aml/aml_logo.img $(BINARIES_DIR)/aml_recovery/ && \
 cp -f $(BINARIES_DIR)/uImage-* $(BINARIES_DIR)/aml_recovery/ && \
 find $(BINARIES_DIR)/aml_recovery/system/ -type l -delete && \
 find $(BINARIES_DIR)/aml_recovery/system/ -type d -empty -exec sh -c 'echo "dummy" > "{}"/.empty' \; && \
 pushd $(BINARIES_DIR)/aml_recovery/ >/dev/null && \
 zip -m -q -r -y $(BINARIES_DIR)/aml_recovery/update-unsigned.img aml_logo.img uImage-2.6.34 META-INF system && \
 popd >/dev/null && \
 echo "Signing update.img..." && \
 pushd fs/recovery_aml/ >/dev/null; java -Xmx1024m -jar signapk.jar -w testkey.x509.pem testkey.pk8 $(BINARIES_DIR)/aml_recovery/update-unsigned.img $(BINARIES_DIR)/update.img && \
 rm -rf $(BINARIES_DIR)/aml_recovery; rm -f $(TARGET_DIR)/usr.sqsh
endef

$(eval $(call ROOTFS_TARGET,recovery_aml))
