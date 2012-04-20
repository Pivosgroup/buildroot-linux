#############################################################
#
# samba
#
#############################################################
SAMBA30_VERSION:=3.0.37
SAMBA30_SOURCE:=samba-$(SAMBA30_VERSION).tar.gz
SAMBA30_SITE:=http://samba.org/samba/ftp/stable

SAMBA30_SUBDIR = source
SAMBA30_AUTORECONF = NO

SAMBA30_INSTALL_STAGING = YES
SAMBA30_INSTALL_TARGET = YES


SAMBA30_DEPENDENCIES = \
	$(if $(BR2_ENABLE_LOCALE),,libiconv) \
	$(if $(BR2_PACKAGE_SAMBA30_AVAHI),avahi) \


SAMBA30_CONF_ENV = \
	SMB_BUILD_CC_NEGATIVE_ENUM_VALUES=yes \
	libreplace_cv_READDIR_GETDIRENTRIES=no \
	libreplace_cv_READDIR_GETDENTS=no \
	linux_getgrouplist_ok=no \
	samba_cv_REPLACE_READDIR=no \
	samba_cv_HAVE_WRFILE_KEYTAB=yes \
	samba_cv_HAVE_GETTIMEOFDAY_TZ=yes \
	samba_cv_USE_SETREUID=yes \
	samba_cv_HAVE_KERNEL_OPLOCKS_LINUX=yes \
	samba_cv_HAVE_IFACE_IFCONF=yes \
	samba_cv_HAVE_MMAP=yes \
	samba_cv_HAVE_FCNTL_LOCK=yes \
	samba_cv_HAVE_SECURE_MKSTEMP=yes \
	samba_cv_CC_NEGATIVE_ENUM_VALUES=yes \
	samba_cv_fpie=no \
        samba_cv_have_longlong=yes \
        samba_cv_HAVE_OFF64_T=yes \
	libreplace_cv_HAVE_IPV6=$(if $(BR2_INET_IPV6),yes,no) \
	$(if $(BR2_PACKAGE_SAMBA30_AVAHI),AVAHI_LIBS=-pthread)


SAMBA30_CONF_OPT = \
	--localstatedir=/var \
	--with-piddir=/var/run \
	--with-lockdir=/var/lock \
	--with-logfilebase=/var/log \
	--with-configdir=/etc/samba \
	--with-privatedir=/etc/samba \
	\
	--disable-cups \
	--enable-static \
	--enable-shared \
	--disable-shared-libs \
	--disable-pie \
	--disable-iprint \
	--disable-relro \
	--disable-dnssd \
	\
	$(if $(BR2_PACKAGE_SAMBA30_AVAHI),--enable-avahi,--disable-avahi) \
	\
        --disable-fam \
        --disable-swat \
	--without-cluster-support \
	--without-cifsupcall \
	--without-ads \
	--without-ldap \
	--with-included-popt \
	--with-included-iniparser \
	--without-sys-quotas \
	--without-krb5 \
	--without-automount \
	--without-sendfile-support \
	--with-libiconv=$(STAGING_DIR) \
        --without-cifsmount \
        --without-winbind \


SAMBA30_INSTALL_TARGET_OPT = \
	DESTDIR=$(TARGET_DIR) -C $(SAMBA30_DIR)/$(SAMBA30_SUBDIR) \
	installclientlib

SAMBA30_INSTALL_STAGING_OPT = \
        DESTDIR=$(STAGING_DIR) -C $(SAMBA30_DIR)/$(SAMBA30_SUBDIR) \
        installclientlib

SAMBA30_UNINSTALL_TARGET_OPT = \
	DESTDIR=$(TARGET_DIR) -C $(SAMBA30_DIR)/$(SAMBA30_SUBDIR) \
	uninstallclientlib

SAMBA30_UNINSTALL_STAGING_OPT = \
        DESTDIR=$(STAGING_DIR) -C $(SAMBA30_DIR)/$(SAMBA30_SUBDIR) \
        uninstallclientlib

# non-binaries to remove
SAMBA30_TXTTARGETS_ = \
	usr/include/libsmbclient.h \
	usr/include/netapi.h \
	usr/include/smb_share_modes.h \
	usr/include/talloc.h \
	usr/include/tdb.h \
	usr/include/wbclient.h

define SAMBA30_REMOVE_UNNEEDED_HEADERS
       rm -f $(addprefix $(TARGET_DIR)/, $(SAMBA30_TXTTARGETS_))
endef

SAMBA30_POST_INSTALL_TARGET_HOOKS += SAMBA30_REMOVE_UNNEEDED_HEADERS

define SAMBA30_AUTOGEN
	@$(call MESSAGE,"Reconfiguring")
        ( cd $(@D)/source && ./autogen.sh )
endef

SAMBA30_PRE_CONFIGURE_HOOKS = SAMBA30_AUTOGEN

$(eval $(call AUTOTARGETS,package/thirdparty,samba30))

SAMBA30_CONFIGURE_CMDS += && ( cd $(@D)/source && make proto )
