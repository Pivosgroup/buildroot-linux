#############################################################
#
# jpeg (libraries needed by some apps)
#
#############################################################
JPEG_VERSION = 1.2.1
JPEG_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/libjpeg-turbo
JPEG_SOURCE = libjpeg-turbo-$(JPEG_VERSION).tar.gz
JPEG_INSTALL_STAGING = YES
JPEG_INSTALL_TARGET = YES
JPEG_CONF_OPT = --program-prefix= --with-jpeg8
JPEG_CFLAGS += -O3
define JPEG_REMOVE_USELESS_TOOLS
	rm -f $(addprefix $(TARGET_DIR)/usr/bin/,cjpeg djpeg jpegtrans rdjpgcom wrjpgcom)
endef

JPEG_POST_INSTALL_TARGET_HOOKS += JPEG_REMOVE_USELESS_TOOLS

$(eval $(call AUTOTARGETS,package,jpeg))
$(eval $(call AUTOTARGETS,package,jpeg,host))
