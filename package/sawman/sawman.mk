#############################################################
#
# SAWMAN
#
#############################################################
SAWMAN_VERSION = 1.4.15
SAWMAN_SOURCE = SaWMan-$(SAWMAN_VERSION).tar.gz
SAWMAN_SITE = http://www.directfb.org/downloads/Extras
SAWMAN_INSTALL_STAGING = YES
SAWMAN_DEPENDENCIES = directfb

$(eval $(call AUTOTARGETS))
