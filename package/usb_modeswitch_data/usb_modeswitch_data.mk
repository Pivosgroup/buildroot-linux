#############################################################
#
# usb_modeswitch_data
#
#############################################################

USB_MODESWITCH_DATA_VERSION = 20120815
USB_MODESWITCH_DATA_SOURCE = usb-modeswitch-data-$(USB_MODESWITCH_DATA_VERSION).tar.bz2
USB_MODESWITCH_DATA_SITE = http://www.draisberghof.de/usb_modeswitch
USB_MODESWITCH_DATA_DEPENDENCIES = usb_modeswitch
USB_MODESWITCH_DATA_LICENSE = GPLv2+
USB_MODESWITCH_DATA_LICENSE_FILES = COPYING

# Nothing to build, it is a pure data package

define USB_MODESWITCH_DATA_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) install
endef

define USB_MODESWITCH_DATA_CLEAN_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) clean
endef

$(eval $(generic-package))
