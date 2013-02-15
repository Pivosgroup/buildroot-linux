#############################################################
#
# ntfs-3g
#
#############################################################

NTFS_3G_VERSION = 2013.1.13
NTFS_3G_SOURCE = ntfs-3g_ntfsprogs-$(NTFS_3G_VERSION).tgz
NTFS_3G_SITE = http://tuxera.com/opensource
NTFS_3G_CONF_OPT = --disable-ldconfig --program-prefix=""
NTFS_3G_INSTALL_STAGING = YES

define NTFS_3G_TARGET_SYMLINK_CREATE
	cd $(TARGET_DIR)/sbin; ln -fs mount.ntfs-3g mount.ntfs
endef

NTFS_3G_POST_INSTALL_TARGET_HOOKS += NTFS_3G_TARGET_SYMLINK_CREATE

$(eval $(call AUTOTARGETS,package,ntfs-3g))
