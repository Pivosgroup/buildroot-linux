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

FRIBIDI_CONF_OPT = --disable-docs --with-glib=no

$(eval $(call AUTOTARGETS,package/thirdparty,fribidi))
