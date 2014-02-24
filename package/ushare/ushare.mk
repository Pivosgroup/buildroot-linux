#############################################################
#
# ushare
#
#############################################################

USHARE_VERSION = 1.1a
USHARE_SOURCE = ushare-$(USHARE_VERSION).tar.bz2
USHARE_SITE = http://ushare.geexbox.org/releases
USHARE_DEPENDENCIES = host-pkg-config libupnp

ifeq ($(BR2_NEEDS_GETTEXT_IF_LOCALE),y)
USHARE_DEPENDENCIES += gettext libintl
USHARE_LDFLAGS += -lintl
endif

define USHARE_CONFIGURE_CMDS
	(cd $(@D); \
		$(TARGET_CONFIGURE_OPTS) \
		./configure --prefix=/usr $(DISABLE_NLS) --cross-compile \
		--cross-prefix="$(TARGET_CROSS)" --sysconfdir=/etc \
		--disable-strip \
	)
endef

define USHARE_BUILD_CMDS
	$(MAKE) LDFLAGS="$(TARGET_LDFLAGS) $(USHARE_LDFLAGS)" -C $(@D)
endef

define USHARE_INSTALL_TARGET_CMDS
	$(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) install
	rm -f $(TARGET_DIR)/etc/init.d/ushare
endef

# Even though configure is called it's not autoconf
$(eval $(call GENTARGETS))
