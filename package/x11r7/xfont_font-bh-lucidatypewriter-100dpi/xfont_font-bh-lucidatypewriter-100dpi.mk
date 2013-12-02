################################################################################
#
# font-bh-lucidatypewriter-100dpi -- No description available
#
################################################################################

XFONT_FONT_BH_LUCIDATYPEWRITER_100DPI_VERSION = 1.0.1
XFONT_FONT_BH_LUCIDATYPEWRITER_100DPI_SOURCE = font-bh-lucidatypewriter-100dpi-$(XFONT_FONT_BH_LUCIDATYPEWRITER_100DPI_VERSION).tar.bz2
XFONT_FONT_BH_LUCIDATYPEWRITER_100DPI_SITE = http://xorg.freedesktop.org/releases/individual/font
XFONT_FONT_BH_LUCIDATYPEWRITER_100DPI_INSTALL_STAGING_OPT = DESTDIR=$(STAGING_DIR) MKFONTSCALE=$(HOST_DIR)/usr/bin/mkfontscale MKFONTDIR=$(HOST_DIR)/usr/bin/mkfontdir install
XFONT_FONT_BH_LUCIDATYPEWRITER_100DPI_INSTALL_TARGET_OPT = DESTDIR=$(TARGET_DIR) MKFONTSCALE=$(HOST_DIR)/usr/bin/mkfontscale MKFONTDIR=$(HOST_DIR)/usr/bin/mkfontdir install-data
XFONT_FONT_BH_LUCIDATYPEWRITER_100DPI_DEPENDENCIES = xfont_font-util host-xfont_font-util host-xapp_mkfontscale host-xapp_mkfontdir host-xapp_bdftopcf

$(eval $(call AUTOTARGETS))

