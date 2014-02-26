################################################################################
#
# usb_modeswitch
#
################################################################################

USB_MODESWITCH_VERSION = 2.0.1
USB_MODESWITCH_SOURCE = usb-modeswitch-$(USB_MODESWITCH_VERSION).tar.bz2
USB_MODESWITCH_SITE = http://www.draisberghof.de/usb_modeswitch
USB_MODESWITCH_DEPENDENCIES = libusb
USB_MODESWITCH_LICENSE = GPLv2+
USB_MODESWITCH_LICENSE_FILES = COPYING

USB_MODESWITCH_BUILD_TARGETS = static
USB_MODESWITCH_INSTALL_TARGETS = install-static

ifeq ($(BR2_PACKAGE_TCL)$(BR2_PACKAGE_TCL_SHLIB_ONLY),y)
	USB_MODESWITCH_DEPENDENCIES += tcl
	USB_MODESWITCH_BUILD_TARGETS = script
	USB_MODESWITCH_INSTALL_TARGETS = install-script
endif

define USB_MODESWITCH_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS) -D_GNU_SOURCE -Wall -I." \
		-C $(@D) $(USB_MODESWITCH_BUILD_TARGETS)
endef

define USB_MODESWITCH_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		DESTDIR=$(TARGET_DIR) \
		-C $(@D) $(USB_MODESWITCH_INSTALL_TARGETS)
endef


define USB_MODESWITCH_CLEAN_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) clean
endef

define USB_MODESWITCH_UNINSTALL_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR=$(TARGET_DIR) uninstall
endef

$(eval $(generic-package))
