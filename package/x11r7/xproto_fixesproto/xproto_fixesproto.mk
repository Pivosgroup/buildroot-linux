################################################################################
#
# xproto_fixesproto -- X.Org Fixes protocol headers
#
################################################################################

XPROTO_FIXESPROTO_VERSION = 4.1.1
XPROTO_FIXESPROTO_SOURCE = fixesproto-$(XPROTO_FIXESPROTO_VERSION).tar.bz2
XPROTO_FIXESPROTO_SITE = http://xorg.freedesktop.org/releases/individual/proto
XPROTO_FIXESPROTO_INSTALL_STAGING = YES
XPROTO_FIXESPROTO_INSTALL_TARGET = NO

$(eval $(call AUTOTARGETS))
$(eval $(call AUTOTARGETS,host))
