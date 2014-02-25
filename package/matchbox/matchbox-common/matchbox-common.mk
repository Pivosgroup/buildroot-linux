#############################################################
#
# MatchBox Common
#
#############################################################
MATCHBOX_COMMON_VERSION_MAJOR = 0.9
MATCHBOX_COMMON_VERSION = $(MATCHBOX_COMMON_VERSION_MAJOR).1
MATCHBOX_COMMON_SOURCE = matchbox-common-$(MATCHBOX_COMMON_VERSION).tar.bz2
MATCHBOX_COMMON_SITE = http://downloads.yoctoproject.org/releases/matchbox/matchbox-common/$(MATCHBOX_COMMON_VERSION_MAJOR)
MATCHBOX_COMMON_LICENSE = GPLv2+
MATCHBOX_COMMON_LICENSE_FILES = COPYING
MATCHBOX_COMMON_DEPENDENCIES = matchbox-lib

ifeq ($(strip $(BR2_PACKAGE_MATCHBOX_COMMON_PDA)),y)
	MATCHBOX_COMMON_CONF_OPT += --enable-pda-folders
endif

#############################################################

$(eval $(autotools-package))
