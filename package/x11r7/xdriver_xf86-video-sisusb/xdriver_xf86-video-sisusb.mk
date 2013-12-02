################################################################################
#
# xdriver_xf86-video-sisusb -- SiS USB video driver
#
################################################################################

XDRIVER_XF86_VIDEO_SISUSB_VERSION = 0.9.3
XDRIVER_XF86_VIDEO_SISUSB_SOURCE = xf86-video-sisusb-$(XDRIVER_XF86_VIDEO_SISUSB_VERSION).tar.bz2
XDRIVER_XF86_VIDEO_SISUSB_SITE = http://xorg.freedesktop.org/releases/individual/driver
XDRIVER_XF86_VIDEO_SISUSB_DEPENDENCIES = xserver_xorg-server xproto_fontsproto xproto_randrproto xproto_renderproto xproto_videoproto xproto_xextproto xproto_xineramaproto xproto_xproto

$(eval $(call AUTOTARGETS))
