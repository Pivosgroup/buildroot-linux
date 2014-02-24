#############################################################
#
# gettext
#
#############################################################
GETTEXT_VERSION:=0.16.1
GETTEXT_SOURCE:=gettext-$(GETTEXT_VERSION).tar.gz
GETTEXT_SITE:=$(BR2_GNU_MIRROR)/gettext
GETTEXT_DIR:=$(BUILD_DIR)/gettext-$(GETTEXT_VERSION)
GETTEXT_CAT:=$(ZCAT)
GETTEXT_BINARY:=gettext-runtime/src/gettext
GETTEXT_TARGET_BINARY:=usr/bin/gettext

ifeq ($(BR2_PACKAGE_GETTEXT_STATIC),y)
LIBINTL_TARGET_BINARY:=usr/lib/libintl.a
else
LIBINTL_TARGET_BINARY:=usr/lib/libintl.so
endif

$(DL_DIR)/$(GETTEXT_SOURCE):
	 $(call DOWNLOAD,$(GETTEXT_SITE)/$(GETTEXT_SOURCE))

gettext-source: $(DL_DIR)/$(GETTEXT_SOURCE)

$(GETTEXT_DIR)/.unpacked: $(DL_DIR)/$(GETTEXT_SOURCE)
	$(GETTEXT_CAT) $(DL_DIR)/$(GETTEXT_SOURCE) | tar -C $(BUILD_DIR) $(TAR_OPTIONS) -
	support/scripts/apply-patches.sh $(GETTEXT_DIR) package/gettext/ gettext\*.patch
	$(call CONFIG_UPDATE,$(@D))
	touch $@

ifneq ($(BR2_TOOLCHAIN_BUILDROOT),y)
IGNORE_EXTERNAL_GETTEXT:=--with-included-gettext
endif

$(GETTEXT_DIR)/.configured: $(GETTEXT_DIR)/.unpacked
	(cd $(GETTEXT_DIR); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CONFIGURE_ARGS) \
		ac_cv_func_strtod=yes \
		ac_fsusage_space=yes \
		fu_cv_sys_stat_statfs2_bsize=yes \
		ac_cv_func_closedir_void=no \
		ac_cv_func_getloadavg=no \
		ac_cv_lib_util_getloadavg=no \
		ac_cv_lib_getloadavg_getloadavg=no \
		ac_cv_func_getgroups=yes \
		ac_cv_func_getgroups_works=yes \
		ac_cv_func_chown_works=yes \
		ac_cv_have_decl_euidaccess=no \
		ac_cv_func_euidaccess=no \
		ac_cv_have_decl_strnlen=yes \
		ac_cv_func_strnlen_working=yes \
		ac_cv_func_lstat_dereferences_slashed_symlink=yes \
		ac_cv_func_lstat_empty_string_bug=no \
		ac_cv_func_stat_empty_string_bug=no \
		vb_cv_func_rename_trailing_slash_bug=no \
		ac_cv_have_decl_nanosleep=yes \
		jm_cv_func_nanosleep_works=yes \
		gl_cv_func_working_utimes=yes \
		ac_cv_func_utime_null=yes \
		ac_cv_have_decl_strerror_r=yes \
		ac_cv_func_strerror_r_char_p=no \
		jm_cv_func_svid_putenv=yes \
		ac_cv_func_getcwd_null=yes \
		ac_cv_func_getdelim=yes \
		ac_cv_func_mkstemp=yes \
		utils_cv_func_mkstemp_limitations=no \
		utils_cv_func_mkdir_trailing_slash_bug=no \
		jm_cv_func_gettimeofday_clobber=no \
		gl_cv_func_working_readdir=yes \
		jm_ac_cv_func_link_follows_symlink=no \
		utils_cv_localtime_cache=no \
		ac_cv_struct_st_mtim_nsec=no \
		gl_cv_func_tzset_clobber=no \
		gl_cv_func_getcwd_null=yes \
		gl_cv_func_getcwd_path_max=yes \
		ac_cv_func_fnmatch_gnu=yes \
		am_getline_needs_run_time_check=no \
		am_cv_func_working_getline=yes \
		gl_cv_func_mkdir_trailing_slash_bug=no \
		gl_cv_func_mkstemp_limitations=no \
		ac_cv_func_working_mktime=yes \
		jm_cv_func_working_re_compile_pattern=yes \
		ac_use_included_regex=no \
		gl_cv_c_restrict=no \
		./configure $(QUIET) \
		--target=$(GNU_TARGET_NAME) \
		--host=$(GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--prefix=/usr \
		--exec-prefix=/usr \
		--disable-libasprintf \
		--enable-shared \
		$(IGNORE_EXTERNAL_GETTEXT) \
		--disable-openmp \
	)
	touch $@

$(GETTEXT_DIR)/$(GETTEXT_BINARY): $(GETTEXT_DIR)/.configured
	$(MAKE) -C $(GETTEXT_DIR)
	touch -c $(GETTEXT_DIR)/$(GETTEXT_BINARY)

$(STAGING_DIR)/$(GETTEXT_TARGET_BINARY): $(GETTEXT_DIR)/$(GETTEXT_BINARY)
	$(MAKE) DESTDIR=$(STAGING_DIR) -C $(GETTEXT_DIR) install
	$(SED) 's,/lib/,$(STAGING_DIR)/usr/lib/,g' $(STAGING_DIR)/usr/lib/libgettextlib.la
	$(SED) 's,/lib/,$(STAGING_DIR)/usr/lib/,g' $(STAGING_DIR)/usr/lib/libgettextpo.la
	$(SED) 's,/lib/,$(STAGING_DIR)/usr/lib/,g' $(STAGING_DIR)/usr/lib/libgettextsrc.la
	$(SED) "s,^libdir=.*,libdir=\'$(STAGING_DIR)/usr/lib\',g" $(STAGING_DIR)/usr/lib/libgettextlib.la
	$(SED) "s,^libdir=.*,libdir=\'$(STAGING_DIR)/usr/lib\',g" $(STAGING_DIR)/usr/lib/libgettextpo.la
	$(SED) "s,^libdir=.*,libdir=\'$(STAGING_DIR)/usr/lib\',g" $(STAGING_DIR)/usr/lib/libgettextsrc.la
	$(SED) "s,^libdir=.*,libdir=\'$(STAGING_DIR)/usr/lib\',g" $(STAGING_DIR)/usr/lib/libintl.la
	rm -f $(addprefix $(STAGING_DIR)/usr/bin/, \
		autopoint envsubst gettext.sh gettextize msg* ?gettext)
	touch -c $@

gettext: host-pkg-config $(if $(BR2_PACKAGE_LIBICONV),libiconv) $(STAGING_DIR)/$(GETTEXT_TARGET_BINARY)

gettext-unpacked: $(GETTEXT_DIR)/.unpacked

gettext-clean:
	-$(MAKE) DESTDIR=$(STAGING_DIR) CC="$(TARGET_CC)" -C $(GETTEXT_DIR) uninstall
	-$(MAKE) DESTDIR=$(TARGET_DIR) CC="$(TARGET_CC)" -C $(GETTEXT_DIR) uninstall
	-$(MAKE) -C $(GETTEXT_DIR) clean

gettext-dirclean:
	rm -rf $(GETTEXT_DIR)

#############################################################
#
# gettext on the target
#
#############################################################

gettext-target: $(GETTEXT_DIR)/$(GETTEXT_BINARY)
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(GETTEXT_DIR) install
	chmod +x $(TARGET_DIR)/usr/lib/libintl.so* # identify as needing to be stripped

$(TARGET_DIR)/usr/lib/libintl.so: $(STAGING_DIR)/$(GETTEXT_TARGET_BINARY)
	cp -dpf $(STAGING_DIR)/usr/lib/libgettext*.so* \
		$(STAGING_DIR)/usr/lib/libintl*.so* $(TARGET_DIR)/usr/lib/
	$(STRIPCMD) $(STRIP_STRIP_UNNEEDED) $(TARGET_DIR)/usr/lib/libgettext*.so*
	$(STRIPCMD) $(STRIP_STRIP_UNNEEDED) $(TARGET_DIR)/usr/lib/libintl*.so*
	rm -f $(addprefix $(TARGET_DIR)/usr/lib/, \
		libgettext*.so*.la libintl*.so*.la)
	touch -c $@

$(TARGET_DIR)/usr/lib/libintl.a: $(STAGING_DIR)/$(GETTEXT_TARGET_BINARY)
	cp -dpf $(STAGING_DIR)/usr/lib/libgettext*.a $(TARGET_DIR)/usr/lib/
	cp -dpf $(STAGING_DIR)/usr/lib/libintl*.a $(TARGET_DIR)/usr/lib/
	touch -c $@

libintl: $(TARGET_DIR)/$(LIBINTL_TARGET_BINARY)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_LIBINTL),y)
TARGETS+=libintl
endif
ifeq ($(BR2_PACKAGE_GETTEXT),y)
TARGETS+=gettext
endif
