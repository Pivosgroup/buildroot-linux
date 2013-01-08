#############################################################
#
# recovery_reboot
#
#############################################################
RECOVERY_REBOOT_VERSION:=0.9.9
RECOVERY_REBOOT_SOURCE=recovery_reboot-$(RECOVERY_REBOOT_VERSION).tar.gz
RECOVERY_REBOOT_SITE=./package/thirdparty/recovery_reboot/src
RECOVERY_REBOOT_SITE_METHOD=cp
RECOVERY_REBOOT_DEPENDENCIES += busybox #install over busybox's 'reboot'

define RECOVERY_REBOOT_BUILD_CMDS
	CC="$(TARGET_CC)" $(MAKE) -C $(@D)
endef

define RECOVERY_REBOOT_INSTALL_TARGET_CMDS
	DESTDIR="$(TARGET_DIR)" $(MAKE) -C $(@D) install
        install $(@D)/recoveryflash $(TARGET_DIR)/usr/sbin
endef

$(eval $(call GENTARGETS,package/thirdparty,recovery_reboot))
