#############################################################
#
# distcc
#
#############################################################
DISTCC_VERSION = 2.18.3
DISTCC_SOURCE = distcc-$(DISTCC_VERSION).tar.bz2
DISTCC_SITE = http://distcc.googlecode.com/files/

DISTCC_CONF_OPT = --with-included-popt --without-gtk --without-gnome

define DISTCC_INSTALL_TARGET_CMDS
	install -D $(@D)/distccd $(TARGET_DIR)/usr/bin/distccd
	install -D $(@D)/distcc $(TARGET_DIR)/usr/bin/distcc
endef

define DISTCC_CLEAN_CMDS
	rm -f $(TARGET_DIR)/usr/bin/distcc
	rm -f $(TARGET_DIR)/usr/bin/distccd
	-$(MAKE) -C $(@D) clean
endef

$(eval $(call AUTOTARGETS))
