#############################################################
#
# if_manager
#
#############################################################
IF_MANAGER_VERSION:=1.1.0
IF_MANAGER_DIR=$(BUILD_DIR)/if_manager-$(IF_MANAGER_VERSION)
IF_MANAGER_SOURCE=src
IF_MANAGER_SITE=.

$(IF_MANAGER_DIR)/.unpacked:
	mkdir -p $(IF_MANAGER_DIR)
	cp -arf ./package/if_manager/src/* $(IF_MANAGER_DIR)
	touch $(IF_MANAGER_DIR)/.unpacked

$(IF_MANAGER_DIR)/.configured: $(IF_MANAGER_DIR)/.unpacked
	(cd $(IF_MANAGER_DIR); rm -rf config.cache; \
	    $(TARGET_CONFIGURE_OPTS) \
	    $(TARGET_CONFIGURE_ARGS) \
	    ./configure \
	    --target=$(GNU_TARGET_NAME) \
	    --host=$(GNU_TARGET_NAME) \
	    --build=$(GNU_HOST_NAME) \
	    --prefix=/usr \
	    --sysconfdir=/etc \
	)
	touch $(IF_MANAGER_DIR)/.configured

$(IF_MANAGER_DIR)/if_manager: $(IF_MANAGER_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(IF_MANAGER_DIR)
	touch -c $(IF_MANAGER_DIR)/if_manager

$(TARGET_DIR)/lib/if_manager: $(IF_MANAGER_DIR)/if_manager
	mkdir -p $(STAGING_DIR)/usr/include/ifm
	cp $(IF_MANAGER_DIR)/include/* $(STAGING_DIR)/usr/include/ifm
	cp -a $(IF_MANAGER_DIR)/src/.libs/libifm.so* $(TARGET_DIR)/usr/lib
	cp -a $(IF_MANAGER_DIR)/src/.libs/libifm.so* $(STAGING_DIR)/usr/lib
	touch -c $(TARGET_DIR)/bin/manager

if_manager: $(TARGET_DIR)/lib/if_manager

if_manager-source: $(DL_DIR)/$(IF_MANAGER_SOURCE)

if_manager-clean:
	-$(MAKE) -C $(IF_MANAGER_DIR) clean

if_manager-dirclean:
	rm -rf $(IF_MANAGER_DIR)


#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_IF_MANAGER),y)
TARGETS+=if_manager
endif
