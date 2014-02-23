#############################################################
#
# openchrome
#
#############################################################
XDRIVER_XF86_VIDEO_OPENCHROME_VERSION = 0.2.904
XDRIVER_XF86_VIDEO_OPENCHROME_SOURCE = xf86-video-openchrome-$(XDRIVER_XF86_VIDEO_OPENCHROME_VERSION).tar.bz2
XDRIVER_XF86_VIDEO_OPENCHROME_SITE = http://www.openchrome.org/releases

XDRIVER_XF86_VIDEO_OPENCHROME_DEPENDENCIES = \
	xserver_xorg-server \
	libdrm \
	xlib_libX11 \
	xlib_libXcomposite \
	xlib_libXvMC \
	xproto_fontsproto \
	xproto_glproto \
	xproto_randrproto \
	xproto_renderproto \
	xproto_xextproto \
	xproto_xf86driproto \
	xproto_xproto


XDRIVER_XF86_VIDEO_OPENCHROME_AUTORECONF = YES

$(eval $(call AUTOTARGETS))
