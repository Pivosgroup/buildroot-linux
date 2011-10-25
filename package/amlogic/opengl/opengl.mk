#############################################################
#
# opengl
#
#############################################################
OPENGL_VERSION:=0.9.9
OPENGL_SOURCE=opengl-$(AMADEC_VERSION).tar.gz
OPENGL_SITE=./package/amlogic/opengl/src
OPENGL_SITE_METHOD=cp
OPENGL_INSTALL_STAGING=YES

define OPENGL_INSTALL_STAGING_CMDS
        install -m 755 $(@D)/lib/*.so.* $(STAGING_DIR)/usr/lib
        cp -rf $(@D)/include/* $(STAGING_DIR)/usr/include
endef

define OPENGL_INSTALL_TARGET_CMDS
        install -m 755 $(@D)/lib/*.so.* $(TARGET_DIR)/usr/lib
endef

$(eval $(call GENTARGETS,package/amlogic,opengl))
