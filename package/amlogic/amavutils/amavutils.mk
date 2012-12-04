#############################################################
#
# amavutils
#
#############################################################
define AMAVUTILS_BUILD_CMDS
  $(MAKE) CC="$(TARGET_CC)" -C $(AMAVUTILS_DIR)
endef

define AMAVUTILS_INSTALL_STAGING_CMDS
  install -m 755 $(AMAVUTILS_DIR)/libamavutils.so $(STAGING_DIR)/lib
  install $(AMAVUTILS_DIR)/include/*.h $(STAGING_DIR)/usr/include/
endef

define AMAVUTILS_INSTALL_TARGET_CMDS
  install -m 755 $(AMAVUTILS_DIR)/libamavutils.so $(TARGET_DIR)/lib
endef
