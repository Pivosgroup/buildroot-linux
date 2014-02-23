#############################################################
#
# sysklogd
#
#############################################################
SYSKLOGD_VERSION = 1.5
SYSKLOGD_SOURCE  = sysklogd_$(SYSKLOGD_VERSION).orig.tar.gz
SYSKLOGD_PATCH   = sysklogd_$(SYSKLOGD_VERSION)-6.diff.gz
SYSKLOGD_SITE    = $(BR2_DEBIAN_MIRROR)/debian/pool/main/s/sysklogd

# Override Busybox implementations if Busybox is enabled.
ifeq ($(BR2_PACKAGE_BUSYBOX),y)
SYSKLOGD_DEPENDENCIES = busybox
endif

define SYSKLOGD_DEBIAN_PATCHES
	if [ -d $(@D)/debian/patches ]; then \
		support/scripts/apply-patches.sh $(@D) $(@D)/debian/patches \*.patch; \
	fi
endef

SYSKLOGD_POST_PATCH_HOOKS = SYSKLOGD_DEBIAN_PATCHES

define SYSKLOGD_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D)
endef

define SYSKLOGD_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0500 $(@D)/syslogd $(TARGET_DIR)/usr/sbin/syslogd
	$(INSTALL) -D -m 0500 $(@D)/klogd $(TARGET_DIR)/usr/sbin/klogd
	$(INSTALL) -D -m 0644 $(@D)/sysklogd.8 $(TARGET_DIR)/usr/share/man/man8/sysklogd.8
	$(INSTALL) -D -m 0644 $(@D)/syslogd.8 $(TARGET_DIR)/usr/share/man/man8/syslogd.8
	$(INSTALL) -D -m 0644 $(@D)/syslog.conf.5 $(TARGET_DIR)/usr/share/man/man5/syslog.conf.5
	$(INSTALL) -D -m 0644 $(@D)/klogd.8 $(TARGET_DIR)/usr/share/man/man8/klogd.8
	if [ ! -f $(TARGET_DIR)/etc/syslog.conf ]; then \
		$(INSTALL) -D -m 0644 package/sysklogd/syslog.conf \
			$(TARGET_DIR)/etc/syslog.conf; \
	fi
endef

define SYSKLOGD_UNINSTALL_TARGET_CMDS
	rm -f $(TARGET_DIR)/usr/sbin/syslogd
	rm -f $(TARGET_DIR)/usr/sbin/klogd
	rm -f $(TARGET_DIR)/usr/share/man/man8/sysklogd.8
	rm -f $(TARGET_DIR)/usr/share/man/man8/syslogd.8
	rm -f $(TARGET_DIR)/usr/share/man/man5/syslog.conf.5
	rm -f $(TARGET_DIR)/usr/share/man/man8/klogd.8
	rm -f $(TARGET_DIR)/etc/syslog.conf
endef

define SYSKLOGD_CLEAN_CMDS
	$(MAKE) -C $(@D) clean
endef

$(eval $(call GENTARGETS))
