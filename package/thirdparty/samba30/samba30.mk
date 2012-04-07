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
	$(if $(BR2_PACKAGE_SAMBA30_RPCCLIENT),readline) \
	$(if $(BR2_PACKAGE_SAMBA30_SMBCLIENT),readline) \
	$(if $(BR2_PACKAGE_SAMBA30_AVAHI),avahi) \
	$(if $(BR2_PACKAGE_SAMBA30_GAMIN),gamin)


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
	$(if $(BR2_PACKAGE_SAMBA30_GAMIN),--enable-fam,--disable-fam) \
	$(if $(BR2_PACKAGE_SAMBA30_SWAT),--enable-swat,--disable-swat) \
	\
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
	\
	$(if $(BR2_PACKAGE_SAMBA30_CIFS),--with-cifsmount,--without-cifsmount) \
	$(if $(BR2_PACKAGE_SAMBA30_RPCCLIENT),--with-readline=$(STAGING_DIR)) \
	$(if $(BR2_PACKAGE_SAMBA30_SMBCLIENT),--with-readline=$(STAGING_DIR)) \
	$(if $(BR2_PACKAGE_SAMBA30_WINBINDD),--with-winbind,--without-winbind)

SAMBA30_INSTALL_TARGET_OPT = \
	DESTDIR=$(TARGET_DIR) -C $(SAMBA30_DIR)/$(SAMBA30_SUBDIR) \
	installclientlib installservers installbin installscripts \
	$(if $(BR2_PACKAGE_SAMBA30_CIFS),installcifsmount) \
	$(if $(BR2_PACKAGE_SAMBA30_SWAT),installswat)


SAMBA30_UNINSTALL_TARGET_OPT = \
	DESTDIR=$(TARGET_DIR) -C $(SAMBA30_DIR)/$(SAMBA30_SUBDIR) \
	uninstalllibs uninstallservers uninstallbin uninstallscripts \
	$(if $(BR2_PACKAGE_SAMBA30_CIFS),uninstallcifsmount) \
	$(if $(BR2_PACKAGE_SAMBA30_SWAT),uninstallswat)


# binaries to keep
SAMBA30_BINTARGETS_y = \
	usr/sbin/smbd \
	usr/lib/libtalloc.so \
	usr/lib/libtdb.so


# binaries to remove
SAMBA30_BINTARGETS_ = \
	usr/lib/libnetapi.so* \
	usr/lib/libsmbsharemodes.so*


# binaries to keep or remove
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_CIFS) += usr/sbin/mount.cifs
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_CIFS) += usr/sbin/umount.cifs
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_EVENTLOGADM) += usr/bin/eventlogadm
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_NET) += usr/bin/net
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_NMBD) += usr/sbin/nmbd
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_NMBLOOKUP) += usr/bin/nmblookup
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_NTLM_AUTH) += usr/bin/ntlm_auth
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_PDBEDIT) += usr/bin/pdbedit
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_PROFILES) += usr/bin/profiles
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_RPCCLIENT) += usr/bin/rpcclient
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBCACLS) += usr/bin/smbcacls
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBCLIENT) += usr/bin/smbclient
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBCONTROL) += usr/bin/smbcontrol
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBCQUOTAS) += usr/bin/smbcquotas
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBGET) += usr/bin/smbget
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBLDBTOOLS) += usr/bin/ldbadd
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBLDBTOOLS) += usr/bin/ldbdel
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBLDBTOOLS) += usr/bin/ldbedit
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBLDBTOOLS) += usr/bin/ldbmodify
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBLDBTOOLS) += usr/bin/ldbrename
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBLDBTOOLS) += usr/bin/ldbsearch
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBPASSWD) += usr/bin/smbpasswd
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBSHARESEC) += usr/bin/sharesec
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBSPOOL) += usr/bin/smbspool
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBSTATUS) += usr/bin/smbstatus
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SMBTREE) += usr/bin/smbtree
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_SWAT) += usr/sbin/swat
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_TDB) += usr/bin/tdbbackup
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_TDB) += usr/bin/tdbdump
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_TDB) += usr/bin/tdbtool
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_TESTPARM) += usr/bin/testparm
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_WINBINDD) += usr/sbin/winbindd
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_WBINFO) += usr/bin/wbinfo

# libraries to keep or remove
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_WINBINDD) += usr/lib/libwbclient.so*
SAMBA30_BINTARGETS_$(BR2_PACKAGE_SAMBA30_LIBSMBCLIENT) += usr/lib/libsmbclient.so*


# non-binaries to remove
SAMBA30_TXTTARGETS_ = \
	usr/include/libsmbclient.h \
	usr/include/netapi.h \
	usr/include/smb_share_modes.h \
	usr/include/talloc.h \
	usr/include/tdb.h \
	usr/include/wbclient.h


# non-binaries to keep or remove
SAMBA30_TXTTARGETS_$(BR2_PACKAGE_SAMBA30_FINDSMB) += usr/bin/findsmb
SAMBA30_TXTTARGETS_$(BR2_PACKAGE_SAMBA30_SMBTAR) += usr/bin/smbtar

define SAMBA30_REMOVE_UNNEEDED_BINARIES
	rm -f $(addprefix $(TARGET_DIR)/, $(SAMBA30_BINTARGETS_))
	rm -f $(addprefix $(TARGET_DIR)/, $(SAMBA30_TXTTARGETS_))
endef

SAMBA30_POST_INSTALL_TARGET_HOOKS += SAMBA30_REMOVE_UNNEEDED_BINARIES

define SAMBA30_REMOVE_SWAT_DOCUMENTATION
	# Remove the documentation
	rm -rf $(TARGET_DIR)/usr/swat/help/manpages
	rm -rf $(TARGET_DIR)/usr/swat/help/Samba3*
	rm -rf $(TARGET_DIR)/usr/swat/using_samba/
	# Removing the welcome.html file will make swat default to
	# welcome-no-samba-doc.html
	rm -rf $(TARGET_DIR)/usr/swat/help/welcome.html
endef

ifeq ($(BR2_PACKAGE_SAMBA30_SWAT),y)
ifneq ($(BR2_HAVE_DOCUMENTATION),y)
SAMBA30_POST_INSTALL_TARGET_HOOKS += SAMBA30_REMOVE_SWAT_DOCUMENTATION
endif
endif

define SAMBA30_INSTALL_INITSCRIPTS_CONFIG
	# install start/stop script
	@if [ ! -f $(TARGET_DIR)/etc/init.d/S91smb ]; then \
		$(INSTALL) -m 0755 -D package/samba/S91smb $(TARGET_DIR)/etc/init.d/S91smb; \
	fi
	# install config
	@if [ ! -f $(TARGET_DIR)/etc/samba/smb.conf ]; then \
		$(INSTALL) -m 0755 -D package/samba/simple.conf $(TARGET_DIR)/etc/samba/smb.conf; \
	fi
endef

SAMBA30_POST_INSTALL_TARGET_HOOKS += SAMBA30_INSTALL_INITSCRIPTS_CONFIG

define SAMBA30_INSTALL_NSS_MODULES
	# install nss modules
	$(INSTALL) -m 0644 -D $(SAMBA30_DIR)/$(SAMBA30_SUBDIR)/nsswitch/libnss_wins.so $(TARGET_DIR)/usr/lib/libnss_wins.so
	@ln -sf libnss_wins.so $(TARGET_DIR)/usr/lib/libnss_wins.so.2
	$(INSTALL) -m 0644 -D $(SAMBA30_DIR)/$(SAMBA30_SUBDIR)/nsswitch/libnss_winbind.so $(TARGET_DIR)/usr/lib/libnss_winbind.so
	@ln -sf libnss_winbind.so $(TARGET_DIR)/usr/lib/libnss_winbind.so.2
endef

ifeq ($(BR2_PACKAGE_SAMBA30_WINBINDD),y)
SAMBA30_POST_INSTALL_TARGET_HOOKS += SAMBA30_INSTALL_NSS_MODULES
endif

define SAMBA30_AUTOGEN
	@$(call MESSAGE,"Reconfiguring")
        ( cd $(@D)/source && ./autogen.sh )
endef

SAMBA30_PRE_CONFIGURE_HOOKS = SAMBA30_AUTOGEN

$(eval $(call AUTOTARGETS,package/thirdparty,samba30))

SAMBA30_CONFIGURE_CMDS += && ( cd $(@D)/source && make proto )
