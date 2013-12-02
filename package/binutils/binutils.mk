#############################################################
#
# binutils
#
#############################################################

# Version is set when using buildroot toolchain.
# If not, we do like other packages
BINUTILS_VERSION = $(call qstrip,$(BR2_BINUTILS_VERSION))
ifeq ($(BINUTILS_VERSION),)
BINUTILS_VERSION = 2.21
endif

BINUTILS_SOURCE = binutils-$(BINUTILS_VERSION).tar.bz2
BINUTILS_SITE = $(BR2_GNU_MIRROR)/binutils
ifeq ($(ARCH),avr32)
BINUTILS_SITE = ftp://www.at91.com/pub/buildroot
endif
BINUTILS_EXTRA_CONFIG_OPTIONS = $(call qstrip,$(BR2_BINUTILS_EXTRA_CONFIG_OPTIONS))
BINUTILS_INSTALL_STAGING = YES
BINUTILS_DEPENDENCIES = $(if $(BR2_NEEDS_GETTEXT_IF_LOCALE),gettext libintl)

# We need to specify host & target to avoid breaking ARM EABI
BINUTILS_CONF_OPT = --disable-multilib --disable-werror \
		--host=$(REAL_GNU_TARGET_NAME) \
		--target=$(REAL_GNU_TARGET_NAME) \
		--enable-shared \
		$(BINUTILS_EXTRA_CONFIG_OPTIONS)

# Install binutils after busybox to prefer full-blown utilities
ifeq ($(BR2_PACKAGE_BUSYBOX),y)
BINUTILS_DEPENDENCIES += busybox
endif

# "host" binutils should actually be "cross"
# We just keep the convention of "host utility" for now
HOST_BINUTILS_CONF_OPT = --disable-multilib --disable-werror \
			--target=$(REAL_GNU_TARGET_NAME) \
			--disable-shared --enable-static \
			$(BR2_CONFIGURE_STAGING_SYSROOT) \
			$(BINUTILS_EXTRA_CONFIG_OPTIONS)

# We just want libbfd and libiberty, not the full-blown binutils in staging
define BINUTILS_INSTALL_STAGING_CMDS
	$(MAKE) -C $(@D)/bfd DESTDIR=$(STAGING_DIR) install
	$(MAKE) -C $(@D)/libiberty DESTDIR=$(STAGING_DIR) install
endef

# If we don't want full binutils on target
ifneq ($(BR2_PACKAGE_BINUTILS_TARGET),y)
define BINUTILS_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/bfd DESTDIR=$(TARGET_DIR) install
	$(MAKE) -C $(@D)/libiberty DESTDIR=$(STAGING_DIR) install
endef
endif

$(eval $(call AUTOTARGETS))
$(eval $(call AUTOTARGETS,host))
