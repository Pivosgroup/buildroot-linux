################################################################################
#
# xapp_xfs -- X font server
#
################################################################################

XAPP_XFS_VERSION = 1.0.5
XAPP_XFS_SOURCE = xfs-$(XAPP_XFS_VERSION).tar.bz2
XAPP_XFS_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XFS_DEPENDENCIES = xlib_libFS xlib_libXfont xproto_fontsproto

$(eval $(call AUTOTARGETS))
