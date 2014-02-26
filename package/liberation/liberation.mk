################################################################################
#
# liberation
#
################################################################################

LIBERATION_VERSION = 1.06.0.20100721
LIBERATION_SITE = http://www.fedorahosted.org/releases/l/i/liberation-fonts
LIBERATION_SOURCE = liberation-fonts-ttf-$(LIBERATION_VERSION).tar.gz

LIBERATION_TARGET_DIR = $(TARGET_DIR)/usr/share/fonts/liberation

ifeq ($(BR2_PACKAGE_LIBERATION_MONO),y)
define LIBERATION_INSTALL_MONO
	$(INSTALL) -m 644 $(@D)/LiberationMono*.ttf $(LIBERATION_TARGET_DIR)
endef
endif

ifeq ($(BR2_PACKAGE_LIBERATION_SANS),y)
define LIBERATION_INSTALL_SANS
	$(INSTALL) -m 644 $(@D)/LiberationSans*.ttf $(LIBERATION_TARGET_DIR)
endef
endif

ifeq ($(BR2_PACKAGE_LIBERATION_SERIF),y)
define LIBERATION_INSTALL_SERIF
	$(INSTALL) -m 644 $(@D)/LiberationSerif*.ttf $(LIBERATION_TARGET_DIR)
endef
endif

define LIBERATION_INSTALL_TARGET_CMDS
	mkdir -p $(LIBERATION_TARGET_DIR)
	$(LIBERATION_INSTALL_MONO)
	$(LIBERATION_INSTALL_SANS)
	$(LIBERATION_INSTALL_SERIF)
endef

define LIBERATION_CLEAN_CMDS
	rm -rf $(LIBERATION_TARGET_DIR)
endef

$(eval $(generic-package))
