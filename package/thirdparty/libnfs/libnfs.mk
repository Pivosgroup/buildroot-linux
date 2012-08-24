#############################################################
#
# libnfs
#
#############################################################
LIBNFS_VERSION = libnfs-1.3.0
LIBNFS_SITE = git://github.com/sahlberg/libnfs.git
LIBNFS_INSTALL_STAGING = YES
LIBNFS_INSTALL_TARGET = YES
LIBNFS_AUTORECONF = YES

$(eval $(call AUTOTARGETS,package/thirdparty,libnfs))
