#############################################################
#
# sed
#
#############################################################
SED_VERSION = 4.2.1
SED_SOURCE = sed-$(SED_VERSION).tar.gz
SED_SITE = $(BR2_GNU_MIRROR)/sed

SED_CONF_OPT = --bindir=/usr/bin \
		--libdir=/lib \
		--libexecdir=/usr/lib \
		--sysconfdir=/etc \
		--datadir=/usr/share \
		--localstatedir=/var \
		--mandir=/usr/share/man \
		--infodir=/usr/share/info \
		--include=$(STAGING_DIR)/usr/include

define SED_MOVE_BINARY
	mv $(TARGET_DIR)/usr/bin/sed $(TARGET_DIR)/bin/
endef

SED_POST_INSTALL_TARGET_HOOKS = SED_MOVE_BINARY

$(eval $(call AUTOTARGETS))
