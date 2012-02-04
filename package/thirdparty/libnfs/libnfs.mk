#############################################################
#
# libnfs
#
#############################################################
LIBNFS_VERSION = 0804e67d7a512585cebd3c453e5d05986b8ad218
LIBNFS_SITE = git://github.com/sahlberg/libnfs.git
LIBNFS_INSTALL_STAGING = YES
LIBNFS_INSTALL_TARGET = YES
LIBNFS_AUTORECONF = YES

$(eval $(call AUTOTARGETS,package/thirdpardy,libnfs))
