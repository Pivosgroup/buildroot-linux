################################################################################
#
# xproto_windowswmproto -- No description available
#
################################################################################

XPROTO_WINDOWSWMPROTO_VERSION = 1.0.4
XPROTO_WINDOWSWMPROTO_SOURCE = windowswmproto-$(XPROTO_WINDOWSWMPROTO_VERSION).tar.bz2
XPROTO_WINDOWSWMPROTO_SITE = http://xorg.freedesktop.org/releases/individual/proto
XPROTO_WINDOWSWMPROTO_INSTALL_STAGING = YES
XPROTO_WINDOWSWMPROTO_INSTALL_TARGET = NO

$(eval $(autotools-package))
