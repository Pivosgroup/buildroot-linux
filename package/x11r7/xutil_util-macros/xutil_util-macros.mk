################################################################################
#
# xutil_util-macros -- No description available
#
################################################################################

XUTIL_UTIL_MACROS_VERSION = 1.3.0
XUTIL_UTIL_MACROS_SOURCE = util-macros-$(XUTIL_UTIL_MACROS_VERSION).tar.bz2
XUTIL_UTIL_MACROS_SITE = http://xorg.freedesktop.org/releases/individual/util
XUTIL_UTIL_MACROS_INSTALL_STAGING = YES
XUTIL_UTIL_MACROS_INSTALL_TARGET = NO

$(eval $(call AUTOTARGETS))
$(eval $(call AUTOTARGETS,host))
