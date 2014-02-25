#############################################################
#
# perl
#
#############################################################

PERL_VERSION_MAJOR = 16
PERL_VERSION = 5.$(PERL_VERSION_MAJOR).1
PERL_SITE = http://www.cpan.org/src/5.0
PERL_SOURCE = perl-$(PERL_VERSION).tar.bz2
PERL_LICENSE = Artistic
PERL_LICENSE_FILES = Artistic
PERL_INSTALL_STAGING = YES

PERL_CROSS_VERSION = 0.7
PERL_CROSS_SITE    = http://download.berlios.de/perlcross
PERL_CROSS_SOURCE  = perl-5.$(PERL_VERSION_MAJOR).0-cross-$(PERL_CROSS_VERSION).tar.gz

# We use the perlcross hack to cross-compile perl. It should
# be extracted over the perl sources, so we don't define that
# as a separate package. Instead, it is downloaded and extracted
# together with perl

define PERL_CROSS_DOWNLOAD
	$(call DOWNLOAD,$(PERL_CROSS_SITE)/$(PERL_CROSS_SOURCE))
endef
PERL_POST_DOWNLOAD_HOOKS += PERL_CROSS_DOWNLOAD

define PERL_CROSS_EXTRACT
	$(INFLATE$(suffix $(PERL_CROSS_SOURCE))) $(DL_DIR)/$(PERL_CROSS_SOURCE) | \
	$(TAR) $(TAR_STRIP_COMPONENTS)=1 -C $(@D) $(TAR_OPTIONS) -
endef
PERL_POST_EXTRACT_HOOKS += PERL_CROSS_EXTRACT

ifeq ($(BR2_PACKAGE_BERKELEYDB),y)
    PERL_DEPENDENCIES += berkeleydb
endif
ifeq ($(BR2_PACKAGE_GDBM),y)
    PERL_DEPENDENCIES += gdbm
endif

# Normally, --mode=cross should automatically do the two steps
# below, but it doesn't work for some reason.
PERL_HOST_CONF_OPT = \
	--mode=buildmini \
	--target=$(GNU_TARGET_NAME) \
	--target-arch=$(GNU_TARGET_NAME) \
	--set-target-name=$(GNU_TARGET_NAME)

# We have to override LD, because an external multilib toolchain ld is not
# wrapped to provide the required sysroot options.  We also can't use ccache
# because the configure script doesn't support it.
PERL_CONF_OPT = \
	--mode=target \
	--target=$(GNU_TARGET_NAME) \
	--target-tools-prefix=$(TARGET_CROSS) \
	--prefix=/usr \
	-Dld="$(TARGET_CC_NOCCACHE)" \
	-A ccflags="$(TARGET_CFLAGS)" \
	-A ldflags="$(TARGET_LDFLAGS) -lm" \
	-A mydomain="" \
	-A myhostname="$(BR2_TARGET_GENERIC_HOSTNAME)" \
	-A myuname="Buildroot $(BR2_VERSION_FULL)" \
	-A osname=linux \
	-A osvers=$(LINUX_VERSION) \
	-A perlamdin=root

ifeq ($(shell expr $(PERL_VERSION_MAJOR) % 2), 1)
    PERL_CONF_OPT += -Dusedevel
endif

ifneq ($(BR2_LARGEFILE),y)
    PERL_CONF_OPT += -Uuselargefiles
endif

PERL_MODULES = $(call qstrip,$(BR2_PACKAGE_PERL_MODULES))
ifneq ($(PERL_MODULES),)
PERL_CONF_OPT += --only-mod=$(subst $(space),$(comma),$(PERL_MODULES))
endif

define PERL_CONFIGURE_CMDS
	(cd $(@D); HOSTCC='$(HOSTCC_NOCACHE)' ./configure $(PERL_HOST_CONF_OPT))
	(cd $(@D); ./configure $(PERL_CONF_OPT))
	$(SED) 's/UNKNOWN-/Buildroot $(BR2_VERSION_FULL) /' $(@D)/patchlevel.h
endef

# perlcross's miniperl_top forgets base, which is required by mktables.
# Instead of patching, it's easier to just set PERL5LIB
define PERL_BUILD_CMDS
	PERL5LIB=$(@D)/dist/base/lib $(MAKE1) -C $(@D) perl modules
endef

define PERL_INSTALL_STAGING_CMDS
	PERL5LIB=$(@D)/dist/base/lib $(MAKE1) -C $(@D) DESTDIR="$(STAGING_DIR)" install.perl
endef

PERL_INSTALL_TARGET_GOALS = install.perl
ifeq ($(BR2_HAVE_DOCUMENTATION),y)
PERL_INSTALL_TARGET_GOALS += install.man
endif


define PERL_INSTALL_TARGET_CMDS
	PERL5LIB=$(@D)/dist/base/lib $(MAKE1) -C $(@D) DESTDIR="$(TARGET_DIR)" $(PERL_INSTALL_TARGET_GOALS)
endef

define PERL_CLEAN_CMDS
	-$(MAKE1) -C $(@D) clean
endef

$(eval $(generic-package))
