#############################################################
#
# qextserialport
#
#############################################################

QEXTSERIALPORT_VERSION     = f83b4e7ca922e53
QEXTSERIALPORT_SITE        = https://qextserialport.googlecode.com/git/
QEXTSERIALPORT_SITE_METHOD = git

QEXTSERIALPORT_LICENSE = MIT

QEXTSERIALPORT_DEPENDENCIES = qt

QEXTSERIALPORT_INSTALL_STAGING = YES

define QEXTSERIALPORT_CONFIGURE_CMDS
	(cd $(@D); $(QT_QMAKE))
endef

define QEXTSERIALPORT_BUILD_CMDS
	$(MAKE) -C $(@D)
endef

define QEXTSERIALPORT_INSTALL_STAGING_CMDS
	mkdir -p $(STAGING_DIR)/usr/include/QExtSerialPort
	cp $(@D)/src/*.h $(STAGING_DIR)/usr/include/QExtSerialPort/
	cp $(@D)/src/QExtSerialPort $(STAGING_DIR)/usr/include/QExtSerialPort/
	cp -a $(@D)/*.so* $(STAGING_DIR)/usr/lib/
	cp $(@D)/qextserialport.pc $(STAGING_DIR)/usr/lib/pkgconfig/
endef

define QEXTSERIALPORT_INSTALL_TARGET_CMDS
	cp -a $(@D)/*.so.* $(TARGET_DIR)/usr/lib
endef

$(eval $(generic-package))
