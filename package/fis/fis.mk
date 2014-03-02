################################################################################
#
# fis
#
################################################################################

FIS_SITE = http://svn.chezphil.org/utils/trunk
FIS_SITE_METHOD = svn
FIS_VERSION = 2892

define FIS_BUILD_CMDS
	$(TARGET_CC) $(TARGET_CFLAGS) -std=c99 -o $(@D)/fis \
		$(@D)/fis.c $(@D)/crc.c $(TARGE_LDFLAGS)
endef

define FIS_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/fis $(TARGET_DIR)/sbin/fis
endef

define FIS_UNINSTALL_TARGET_CMDS
	rm -f $(TARGET_DIR)/sbin/fis
endef

$(eval $(generic-package))
