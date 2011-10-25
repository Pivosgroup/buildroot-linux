#############################################################
#
# bzip2
#
#############################################################

GADMEI_SOURCE:=gadmei

$(BUILD_DIR)/$(GADMEI_SOURCE):
	svn co https://svn-bj.amlogic.com/svn/app_gadmei_mbx/trunk $(BUILD_DIR)/$(GADMEI_SOURCE)




$(TARGET_DIR)/gadmei:$(BUILD_DIR)/$(GADMEI_SOURCE) qt amplayer_plugins
	. $(BUILD_DIR)/$(GADMEI_SOURCE)/env_for_build.sh $(BUILD_DIR)/../ $(BUILD_DIR)/$(GADMEI_SOURCE)
	
	cp -rf $(BUILD_DIR)/$(GADMEI_SOURCE) $(TARGET_DIR)/
	rm -rf $(TARGET_DIR)/$(GADMEI_SOURCE)/apps\
		   $(TARGET_DIR)/$(GADMEI_SOURCE)/component \
		   $(TARGET_DIR)/$(GADMEI_SOURCE)/docs \
		   $(TARGET_DIR)/$(GADMEI_SOURCE)/release_tool 
		   
	cp $(BUILD_DIR)/$(GADMEI_SOURCE)/docs/S90gadmei  $(TARGET_DIR)/etc/init.d/			
	
mbx_gadmei:$(TARGET_DIR)/gadmei 
mbx_gadmei-clean:
	. $(BUILD_DIR)/$(GADMEI_SOURCE)/env_for_build.sh clean $(BUILD_DIR)/$(GADMEI_SOURCE)

	
mbx_gadmei-dirclean:
	rm -rf $(BUILD_DIR)/$(GADMEI_SOURCE)
	rm -rf $(TARGET_DIR)/$(GADMEI_SOURCE)
	rm  $(TARGET_DIR)/etc/init.d/S90gadmei	

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_GADMEI),y)
TARGETS+=mbx_gadmei
endif
