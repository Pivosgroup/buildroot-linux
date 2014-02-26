################################################################################
#
# rpi-userland
#
################################################################################

RPI_USERLAND_VERSION = 0d486513f56453f8203c270fcad0b04f8e4f1fee
RPI_USERLAND_SITE = http://github.com/raspberrypi/userland/tarball/$(RPI_USERLAND_VERSION)
RPI_USERLAND_LICENSE = BSD-3c
RPI_USERLAND_LICENSE_FILES = LICENCE
RPI_USERLAND_INSTALL_STAGING = YES
RPI_USERLAND_CONF_OPT = -DVMCS_INSTALL_PREFIX=/usr

define RPI_USERLAND_POST_TARGET_CLEANUP
    rm -Rf $(TARGET_DIR)/usr/src
endef
RPI_USERLAND_POST_INSTALL_TARGET_HOOKS += RPI_USERLAND_POST_TARGET_CLEANUP

$(eval $(cmake-package))
