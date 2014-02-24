#############################################################
#
# tinyhttpd
#
#############################################################
TINYHTTPD_VERSION = 0.1.0
TINYHTTPD_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/tinyhttpd/

define TINYHTTPD_BUILD_CMDS
	$(MAKE) -C $(@D) CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define TINYHTTPD_INSTALL_TARGET_CMDS
	$(INSTALL) -m 0755 -D $(@D)/httpd $(TARGET_DIR)/usr/sbin/tinyhttpd
	$(INSTALL) -m 0755 -D package/tinyhttpd/S85tinyhttpd \
		$(TARGET_DIR)/etc/init.d/S85tinyhttpd
	mkdir -p $(TARGET_DIR)/var/www
endef

define TINYHTTPD_CLEAN_CMDS
	rm -f $(TARGET_DIR)/usr/sbin/tinyhttpd
	rm -f $(TARGET_DIR)/etc/init.d/S85tinyhttpd
endef

$(eval $(call GENTARGETS))
