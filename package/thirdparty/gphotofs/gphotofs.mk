#############################################################
#
# GPHOTOFS
#
#############################################################

GPHOTOFS_VERSION = 0.5
GPHOTOFS_SOURCE = gphotofs-$(GPHOTOFS_VERSION).tar.gz
GPHOTOFS_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/gphoto/gphotofs
GPHOTOFS_INSTALL_STAGING = YES
GPHOTOFS_INSTALL_TARGET = YES
GPHOTOFS_DEPENDENCIES = host-libgphoto2 libusb-compat libfuse libgphoto2 udev
GPHOTOFS_CONF_ENV += LDFLAGS+="-lgphoto2"

define GPHOTOFS_INSTALL_UDEV
        $(HOST_DIR)/usr/lib/libgphoto2/print-camera-list udev-rules version 175 mode 0660 > $(TARGET_DIR)/etc/udev/rules.d/40-gphoto.rules
         cat package/thirdparty/gphotofs/40-gphoto.rules >> $(TARGET_DIR)/etc/udev/rules.d/40-gphoto.rules
        $(INSTALL) -m 0755 package/thirdparty/gphotofs/mountptp.sh $(TARGET_DIR)/usr/bin/mountptp.sh
        $(INSTALL) -m 0755 package/thirdparty/gphotofs/umountptp.sh $(TARGET_DIR)/usr/bin/umountptp.sh
endef

GPHOTOFS_POST_INSTALL_TARGET_HOOKS += GPHOTOFS_INSTALL_UDEV

$(eval $(call AUTOTARGETS,package/thirdparty,gphotofs))
