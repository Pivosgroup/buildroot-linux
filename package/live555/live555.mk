################################################################################
#
# live555
#
################################################################################

LIVE555_VERSION = 2011.06.16
LIVE555_SOURCE = live.$(LIVE555_VERSION).tar.gz
LIVE555_SITE = http://www.live555.com/liveMedia/public/
LIVE555_INSTALL_STAGING = YES

define LIVE555_CONFIGURE_CMDS
	echo 'COMPILE_OPTS = $$(INCLUDES) -I. -DSOCKLEN_T=socklen_t $(TARGET_CFLAGS)' >> $(@D)/config.linux
	echo 'C_COMPILER = $(TARGET_CC)' >> $(@D)/config.linux
	echo 'CPLUSPLUS_COMPILER = $(TARGET_CXX)' >> $(@D)/config.linux
	echo 'LINK = $(TARGET_CXX) -o' >> $(@D)/config.linux
	echo 'LINK_OPTS = -L. $(TARGET_LDFLAGS)' >> $(@D)/config.linux
	(cd $(@D); ./genMakefiles linux)
endef

define LIVE555_BUILD_CMDS
	$(MAKE) -C $(@D) all
endef

define LIVE555_CLEAN_CMDS
	$(MAKE) -C $(@D) clean
endef

LIVE555_HEADERS_TO_INSTALL = \
	liveMedia/include \
	groupsock/include \
	UsageEnvironment/include \
	BasicUsageEnvironment/include

LIVE555_LIBS_TO_INSTALL = \
	liveMedia/libliveMedia.a \
	groupsock/libgroupsock.a \
	UsageEnvironment/libUsageEnvironment.a \
	BasicUsageEnvironment/libBasicUsageEnvironment.a

LIVE555_FILES_TO_INSTALL- =
LIVE555_FILES_TO_INSTALL-y =
LIVE555_FILES_TO_INSTALL-$(BR2_PACKAGE_LIVE555_OPENRTSP) += testProgs/openRTSP
LIVE555_FILES_TO_INSTALL-$(BR2_PACKAGE_LIVE555_MEDIASERVER) += mediaServer/live555MediaServer
LIVE555_FILES_TO_INSTALL-$(BR2_PACKAGE_LIVE555_MPEG2_INDEXER) += testProgs/MPEG2TransportStreamIndexer
LIVE555_FILES_TO_INSTALL- += $(LIVE555_FILES_TO_INSTALL-y)

define LIVE555_INSTALL_STAGING_CMDS
	for i in $(LIVE555_HEADERS_TO_INSTALL); do \
		mkdir -p $(STAGING_DIR)/usr/include/live/`dirname $$i`; \
		cp -a $(@D)/$$i/* $(STAGING_DIR)/usr/include/live/`dirname $$i`; \
	done; \
	for i in $(LIVE555_LIBS_TO_INSTALL); do \
		$(INSTALL) -D -m 0755 $(@D)/$$i $(STAGING_DIR)/usr/lib/`basename $$i`; \
	done
endef

define LIVE555_INSTALL_TARGET_CMDS
	for i in $(LIVE555_FILES_TO_INSTALL-y); do \
		$(INSTALL) -D -m 0755 $(@D)/$$i $(TARGET_DIR)/usr/bin/`basename $$i`; \
	done
endef

define LIVE555_UNINSTALL_STAGING_CMDS
	rm -rf $(STAGING_DIR)/usr/include/live
	for i in $(LIVE555_LIBS_TO_INSTALL); do \
		rm -f $(addprefix $(STAGING_DIR)/usr/lib/, `basename $$i`); \
	done
endef

define LIVE555_UNINSTALL_TARGET_CMDS
	for i in $(LIVE555_FILES_TO_INSTALL-); do \
		rm -f $(addprefix $(TARGET_DIR)/usr/bin/, `basename $$i`); \
	done
endef

$(eval $(generic-package))
