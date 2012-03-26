#############################################################
#
# afpfs-ng
#
#############################################################
AFPFS_NG_VERSION = 0.8.1
AFPFS_NG_SITE = http://mirrors.xbmc.org/build-deps/darwin-libs
AFPFS_NG_SOURCE = afpfs-ng-$(AFPFS_NG_VERSION).tar.bz2
AFPFS_NG_INSTALL_STAGING = YES
AFPFS_NG_INSTALL_TARGET = YES
AFPFS_NG_AUTORECONF = YES

AFPFS_NG_CONF_ENV = \
	ac_cv_func_malloc_0_nonnull=yes

AFPFS_NG_CONF_OPT = \
	--disable-fuse 

$(eval $(call AUTOTARGETS,package/thirdpardy,afpfs-ng))
