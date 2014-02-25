#############################################################
#
# openocd
#
#############################################################
OPENOCD_VERSION:=0.5.0
OPENOCD_SOURCE = openocd-$(OPENOCD_VERSION).tar.bz2
OPENOCD_SITE = http://downloads.sourceforge.net/project/openocd/openocd/$(OPENOCD_VERSION)

OPENOCD_AUTORECONF = YES
OPENOCD_CONF_OPT = \
	--oldincludedir=$(STAGING_DIR)/usr/include \
	--includedir=$(STAGING_DIR)/usr/include \
	--disable-doxygen-html \
	--enable-dummy

OPENOCD_DEPENDENCIES = libusb-compat

# Adapters
ifeq ($(BR2_PACKAGE_OPENOCD_FT2XXX),y)
OPENOCD_CONF_OPT += --enable-ft2232_libftdi
OPENOCD_DEPENDENCIES += libftdi
endif

ifeq ($(BR2_PACKAGE_OPENOCD_JLINK),y)
OPENOCD_CONF_OPT += --enable-jlink
endif

ifeq ($(BR2_PACKAGE_OPENOCD_VSLLINK),y)
OPENOCD_CONF_OPT += --enable-vsllink
endif

HOST_OPENOCD_DEPENDENCIES = host-libusb-compat host-libftdi

HOST_OPENOCD_CONF_OPT = 	\
	--disable-doxygen-html 	\
	--enable-dummy 		\
	--enable-ft2232_libftdi \
	--enable-jlink 		\
	--enable-vsllink

$(eval $(autotools-package))
$(eval $(host-autotools-package))
