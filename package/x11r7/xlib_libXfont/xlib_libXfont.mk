################################################################################
#
# xlib_libXfont
#
################################################################################

XLIB_LIBXFONT_VERSION = 1.4.5
XLIB_LIBXFONT_SOURCE = libXfont-$(XLIB_LIBXFONT_VERSION).tar.bz2
XLIB_LIBXFONT_SITE = http://xorg.freedesktop.org/releases/individual/lib
XLIB_LIBXFONT_LICENSE = MIT
XLIB_LIBXFONT_LICENSE_FILES = COPYING
XLIB_LIBXFONT_AUTORECONF = YES
XLIB_LIBXFONT_INSTALL_STAGING = YES
XLIB_LIBXFONT_DEPENDENCIES = freetype xlib_libfontenc xlib_xtrans xproto_fontsproto xproto_xproto xfont_encodings
XLIB_LIBXFONT_CONF_OPT = --disable-devel-docs

HOST_XLIB_LIBXFONT_CONF_OPT = --disable-devel-docs

$(eval $(autotools-package))
$(eval $(host-autotools-package))
