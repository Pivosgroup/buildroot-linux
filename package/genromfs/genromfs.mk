#############################################################
#
# genromfs
#
#############################################################

GENROMFS_VERSION=0.5.2
GENROMFS_SOURCE=genromfs-$(GENROMFS_VERSION).tar.gz
GENROMFS_SITE=http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/romfs

define GENROMFS_BUILD_CMDS
 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
    CC="$(TARGET_CC)" \
    CFLAGS="$(TARGET_CFLAGS)" \
    LDFLAGS="$(TARGET_LDFLAGS)"
endef

define GENROMFS_INSTALL_TARGET_CMDS
 $(TARGET_MAKE_ENV) $(MAKE) -C $(@D) PREFIX=$(TARGET_DIR) install
endef

define HOST_GENROMFS_BUILD_CMDS
  $(HOST_MAKE_ENV) $(MAKE) -C $(@D)
endef

define HOST_GENROMFS_INSTALL_CMDS
  $(HOST_MAKE_ENV) $(MAKE) -C $(@D) PREFIX=$(HOST_DIR) install
endef

$(eval $(call GENTARGETS))
$(eval $(call GENTARGETS,host))