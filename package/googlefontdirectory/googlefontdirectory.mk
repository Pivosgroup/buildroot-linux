#############################################################
#
# googlefontdirectory
#
#############################################################

GOOGLEFONTDIRECTORY_SITE = \
	https://s3.amazonaws.com/joemaller_google_webfonts
GOOGLEFONTDIRECTORY_SOURCE = googlewebfonts.tgz
GOOGLEFONTDIRECTORY_LICENSE = OFLv1.1

GOOGLEFONTDIRECTORY_FONTS = \
	$(call qstrip,$(BR2_PACKAGE_GOOGLEFONTDIRECTORY_FONTS))

define GOOGLEFONTDIRECTORY_INSTALL_TARGET_CMDS
	for i in $(GOOGLEFONTDIRECTORY_FONTS); \
	do \
		$(INSTALL) -d $(TARGET_DIR)/usr/share/fonts/$$i && \
		$(INSTALL) -m 0644 -t $(TARGET_DIR)/usr/share/fonts/$$i $(@D)/$$i/*.ttf; \
	done
endef

define GOOGLEFONTDIRECTORY_UNINSTALL_TARGET_CMDS
	rm -rf $(addprefix $(TARGET_DIR)/usr/share/fonts/,$(GOOGLEFONTDIRECTORY_FONTS))
endef

$(eval $(generic-package))
