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
	find $(@D)/lib -type f -exec install -m 644 {} $(STAGING_DIR)/usr/lib \;
	cd $(STAGING_DIR)/usr/lib; mv libEGL.so.1.4 libEGL.so
	cd $(STAGING_DIR)/usr/lib; mv libGLESv1_CM.so.1.1 libGLESv1_CM.so
	cd $(STAGING_DIR)/usr/lib; mv libGLESv2.so.2.0 libGLESv2.so
	cd $(STAGING_DIR)/usr/lib; rm -f libGLESv1_CM.so.1 #corrupt file
	cp -rf  $(@D)/include/* $(STAGING_DIR)/usr/include
endef

define OPENGL_INSTALL_TARGET_CMDS
	find $(@D)/lib -type f -exec install -m 644 {} $(TARGET_DIR)/usr/lib \;
	cd $(TARGET_DIR)/usr/lib; mv libEGL.so.1.4 libEGL.so
	cd $(TARGET_DIR)/usr/lib; mv libGLESv1_CM.so.1.1 libGLESv1_CM.so
	cd $(TARGET_DIR)/usr/lib; mv libGLESv2.so.2.0 libGLESv2.so
	cd $(TARGET_DIR)/usr/lib; rm -f libGLESv1_CM.so.1 #corrupt file
endef

$(eval $(call GENTARGETS,package/amlogic,opengl))
