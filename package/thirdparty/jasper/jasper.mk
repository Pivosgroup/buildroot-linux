#############################################################
#
# jasper (libraries needed by some apps)
#
#############################################################
JASPER_VERSION = 1.900.1
JASPER_SITE = http://sources.openelec.tv/devel
JASPER_SOURCE = jasper-$(JASPER_VERSION).tar.bz2
JASPER_INSTALL_STAGING = YES
JASPER_INSTALL_TARGET = YES

$(eval $(call AUTOTARGETS,package/thirdparty,jasper))
