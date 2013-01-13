#############################################################
#
# libcec
#
#############################################################
LIBCEC_VERSION = 97ffee06080535a5507d8fbb3c8941bccc51ae13
LIBCEC_SITE = git://github.com/Pulse-Eight/libcec.git
LIBCEC_INSTALL_STAGING = YES
LIBCEC_INSTALL_TARGET = YES
LIBCEC_AUTORECONF = YES
LIBCEC_DEPENDENCIES += udev
#LIBCEC_MAKE=$(MAKE1)

$(eval $(call AUTOTARGETS,package/thirdparty,libcec))
