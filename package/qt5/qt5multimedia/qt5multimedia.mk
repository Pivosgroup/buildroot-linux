################################################################################
#
# qt5multimedia
#
################################################################################

QT5MULTIMEDIA_VERSION = $(QT5_VERSION)
QT5MULTIMEDIA_SITE = $(QT5_SITE)
QT5MULTIMEDIA_SOURCE = qtmultimedia-opensource-src-$(QT5MULTIMEDIA_VERSION).tar.xz
QT5MULTIMEDIA_DEPENDENCIES = qt5base qt5declarative
QT5MULTIMEDIA_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_QT5BASE_LICENSE_APPROVED),y)
QT5MULTIMEDIA_LICENSE = LGPLv2.1 or GPLv3.0
# Here we would like to get license files from qt5base, but qt5base
# may not be extracted at the time we get the legal-info for
# qt5script.
else
QT5MULTIMEDIA_LICENSE = Commercial license
QT5MULTIMEDIA_REDISTRIBUTE = NO
endif

define QT5MULTIMEDIA_CONFIGURE_CMDS
	(cd $(@D); $(HOST_DIR)/usr/bin/qmake)
endef

define QT5MULTIMEDIA_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define QT5MULTIMEDIA_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) install
	$(QT5_LA_PRL_FILES_FIXUP)
endef

ifeq ($(BR2_PREFER_STATIC_LIB),)
define QT5MULTIMEDIA_INSTALL_TARGET_LIBS
	cp -dpf $(STAGING_DIR)/usr/lib/libQt5Multimedia*.so.* $(TARGET_DIR)/usr/lib
	cp -dpfr $(STAGING_DIR)/usr/lib/qt/plugins/* $(TARGET_DIR)/usr/lib/qt/plugins
endef
endif

define QT5MULTIMEDIA_INSTALL_TARGET_CMDS
	cp -dpfr $(STAGING_DIR)/usr/qml/* $(TARGET_DIR)/usr/qml
	$(QT5MULTIMEDIA_INSTALL_TARGET_LIBS)
endef

$(eval $(generic-package))
