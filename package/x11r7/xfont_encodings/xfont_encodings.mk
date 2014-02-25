################################################################################
#
# xfont_encodings -- No description available
#
################################################################################

XFONT_ENCODINGS_VERSION = 1.0.4
XFONT_ENCODINGS_SOURCE = encodings-$(XFONT_ENCODINGS_VERSION).tar.bz2
XFONT_ENCODINGS_SITE = http://xorg.freedesktop.org/releases/individual/font
XFONT_ENCODINGS_DEPENDENCIES = host-xapp_mkfontscale host-pkgconf xutil_util-macros

$(eval $(autotools-package))
$(eval $(host-autotools-package))
