################################################################################
#
# xdriver_xf86-video-ark
#
################################################################################

XDRIVER_XF86_VIDEO_ARK_VERSION = 0.7.4
XDRIVER_XF86_VIDEO_ARK_SOURCE = xf86-video-ark-$(XDRIVER_XF86_VIDEO_ARK_VERSION).tar.bz2
XDRIVER_XF86_VIDEO_ARK_SITE = http://xorg.freedesktop.org/releases/individual/driver
XDRIVER_XF86_VIDEO_ARK_LICENSE = MIT
XDRIVER_XF86_VIDEO_ARK_LICENSE_FILES = COPYING
XDRIVER_XF86_VIDEO_ARK_DEPENDENCIES = xserver_xorg-server xproto_fontsproto xproto_randrproto xproto_renderproto xproto_xextproto xproto_xproto

$(eval $(autotools-package))
