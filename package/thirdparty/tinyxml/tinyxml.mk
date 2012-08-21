#############################################################
#
# tinyxml (libraries needed by some apps)
#
#############################################################
TINYXML_VERSION = 2.6.2_2
TINYXML_SOURCE = tinyxml-$(TINYXML_VERSION).tar.gz
TINYXML_SITE = http://mirrors.xbmc.org/build-deps/sources
TINYXML_AUTORECONF = YES
TINYXML_INSTALL_STAGING = YES
TINYXML_INSTALL_TARGET = YES
$(eval $(call AUTOTARGETS,package/thirdparty,tinyxml))
