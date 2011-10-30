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
	install -m 644 $(@D)/lib/*.so* $(STAGING_DIR)/usr/lib
	cd $(STAGING_DIR)/usr/lib; ln -sf libEGL.so.1.4 libEGL.so.1
	cd $(STAGING_DIR)/usr/lib; ln -sf libEGL.so.1 libEGL.so
	cd $(STAGING_DIR)/usr/lib; ln -sf libGLESv1_CM.so.1.1 libGLESv1_CM.so.1
	cd $(STAGING_DIR)/usr/lib; ln -sf libGLESv1_CM.so.1 libGLESv1_CM.so
	cd $(STAGING_DIR)/usr/lib; ln -sf libGLESv2.so.2.0 libGLESv2.so.2
	cd $(STAGING_DIR)/usr/lib; ln -sf libGLESv2.so.2 libGLESv2.so
	cp -rf  $(@D)/include/* $(STAGING_DIR)/usr/include
endef

define OPENGL_INSTALL_TARGET_CMDS
	install -m 644 $(@D)/lib/*.so* $(STAGING_DIR)/usr/lib
endef

$(eval $(call GENTARGETS,package/amlogic,opengl))
