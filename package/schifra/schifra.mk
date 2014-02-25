#############################################################
#
# schifra
#
#############################################################
SCHIFRA_VERSION = 0.0.1
SCHIFRA_SITE = http://www.schifra.com/downloads
SCHIFRA_SOURCE = schifra.tgz
SCHIFRA_INSTALL_STAGING = YES
SCHIFRA_LICENSE = schifra license
SCHIFRA_LICENSE_FILES = schifra_license.txt

SCHIFRA_MAKE_OPT = COMPILER="$(TARGET_CXX)" \
		   OPTIONS="$(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o"

# The examples are the only buildable artefacts.
ifeq ($(BR2_PACKAGE_SCHIFRA_EXAMPLES),y)
define SCHIFRA_BUILD_CMDS
	$(MAKE) -C $(@D) $(SCHIFRA_MAKE_OPT) all
endef

define SCHIFRA_INSTALL_EXAMPLES
	cd $(@D) && for i in `find -type f -name 'schifra_*' -executable` ; \
	do \
		$(INSTALL) -m 0755 -D $$i (TARGET_DIR)/usr/bin/$$i; \
	done
endef

SCHIFRA_POST_TARGET_INSTALL_HOOKS += SCHIFRA_INSTALL_EXAMPLES
endif

define SCHIFRA_INSTALL_TARGET_CMDS
	cd $(@D) && for i in schifra_*.hpp; do \
		$(INSTALL) -m 0644 -D $$i $(TARGET_DIR)/usr/include/$$i; done
endef

define SCHIFRA_INSTALL_STAGING_CMDS
	cd $(@D) && for i in schifra_*.hpp; do \
		$(INSTALL) -m 0644 -D $$i $(STAGING_DIR)/usr/include/$$i; done
endef

define SCHIFRA_UNINSTALL_TARGET_CMDS
	$(RM) $(TARGET_DIR)/usr/include/schifra_*.hpp
	$(RM) $(TARGET_DIR)/usr/bin/schifra_*
endef

define SCHIFRA_UNINSTALL_STAGING_CMDS
	$(RM) $(STAGING_DIR)/usr/include/schifra_*.hpp
endef

define SCHIFRA_CLEAN_CMDS
	$(MAKE) -C $(@D) $(SCHIFRA_MAKE_OPT) clean
endef

$(eval $(generic-package))
