################################################################################
#
# font-bitstream-75dpi -- No description available
#
################################################################################

XFONT_FONT_BITSTREAM_75DPI_VERSION = 1.0.1
XFONT_FONT_BITSTREAM_75DPI_SOURCE = font-bitstream-75dpi-$(XFONT_FONT_BITSTREAM_75DPI_VERSION).tar.bz2
XFONT_FONT_BITSTREAM_75DPI_SITE = http://xorg.freedesktop.org/releases/individual/font
XFONT_FONT_BITSTREAM_75DPI_INSTALL_STAGING_OPT = DESTDIR=$(STAGING_DIR) MKFONTSCALE=$(HOST_DIR)/usr/bin/mkfontscale MKFONTDIR=$(HOST_DIR)/usr/bin/mkfontdir install
XFONT_FONT_BITSTREAM_75DPI_INSTALL_TARGET_OPT = DESTDIR=$(TARGET_DIR) MKFONTSCALE=$(HOST_DIR)/usr/bin/mkfontscale MKFONTDIR=$(HOST_DIR)/usr/bin/mkfontdir install-data
XFONT_FONT_BITSTREAM_75DPI_DEPENDENCIES = xfont_font-util host-xfont_font-util host-xapp_mkfontscale host-xapp_mkfontdir host-xapp_bdftopcf

$(eval $(call AUTOTARGETS))

