V4LUTILS_VERSION = 0.8.8
V4LUTILS_SOURCE = v4l-utils-$(V4LUTILS_VERSION).tar.bz2
V4LUTILS_SITE = http://linuxtv.org/downloads/v4l-utils
V4LUTILS_INSTALL_STAGING = YES
V4LUTILS_INSTALL_TARGET = YES
V4LUTILS_DEPENDENCIES = jpeg

define V4LUTILS_BUILD_CMDS
	# v4l tries to build a qt app with all the wrong paths if qt is installed. Make sure it doesn't find it.
	sed -i "s/qmake/qmake-none/g" $(@D)/utils/Makefile
        make DESTDIR="$(STAGING_DIR)" PREFIX="/usr" CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" -C $(@D)
endef

define V4LUTILS_INSTALL_STAGING_CMDS
	make DESTDIR="$(STAGING_DIR)" PREFIX="/usr" CC="$(TARGET_CC)" CXX="$(TARGET_CXX)" -C $(@D) install
endef

define V4LUTILS_INSTALL_TARGET_CMDS
        make DESTDIR="$(TARGET_DIR)" PREFIX="/usr" -C $(@D) install
endef

$(eval $(call GENTARGETS,package/thirdparty,v4lutils))
