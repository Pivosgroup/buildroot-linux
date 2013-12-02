#############################################################
#
# pptp-linux
#
#############################################################

PPTP_LINUX_VERSION = 1.7.2
PPTP_LINUX_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/pptpclient
PPTP_LINUX_SOURCE = pptp-$(PPTP_LINUX_VERSION).tar.gz

define PPTP_LINUX_BUILD_CMDS
	$(MAKE) -C $(@D) OPTIMIZE= DEBUG= \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define PPTP_LINUX_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/pptp $(TARGET_DIR)/usr/sbin/pptp
	$(INSTALL) -m 0644 -D $(@D)/pptp.8 $(TARGET_DIR)/usr/share/man/man8/pptp.8
endef

define PPTP_LINUX_UNINSTALL_TARGET_CMDS
	rm -f $(TARGET_DIR)/usr/sbin/pptp
	rm -f $(TARGET_DIR)/usr/share/man/man8/pptp.8
endef

$(eval $(call GENTARGETS))
