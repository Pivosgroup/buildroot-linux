#############################################################
#
# fribidi
#
#############################################################
FRIBIDI_VERSION = 0.19.1
FRIBIDI_SOURCE = fribidi-$(FRIBIDI_VERSION).tar.gz
FRIBIDI_SITE = http://fribidi.org/download
FRIBIDI_INSTALL_STAGING = YES
FRIBIDI_INSTALL_TARGET = YES

$(eval $(call AUTOTARGETS,package/thirdparty,fribidi))
