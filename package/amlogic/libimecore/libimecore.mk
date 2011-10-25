#############################################################
#
# libimecore
#
#############################################################
LIBIMECORE_VERSION:=1.0.0
LIBIMECORE_DIR=$(BUILD_DIR)/libimecore-$(LIBIMECORE_VERSION)
LIBIMECORE_SOURCE=src
LIBIMECORE_SITE=.

$(LIBIMECORE_DIR)/.unpacked:
	mkdir -p $(LIBIMECORE_DIR)
	cp -arf ./package/libimecore/src/* $(LIBIMECORE_DIR)
	touch $(LIBIMECORE_DIR)/.unpacked

$(LIBIMECORE_DIR)/.configured: $(LIBIMECORE_DIR)/.unpacked
	(cd $(LIBIMECORE_DIR); rm -rf config.cache; \
	    $(TARGET_CONFIGURE_OPTS) \
	    $(TARGET_CONFIGURE_ARGS) \
	    ./configure \
	    --target=$(GNU_TARGET_NAME) \
	    --host=$(GNU_TARGET_NAME) \
	    --build=$(GNU_HOST_NAME) \
	    --prefix=/usr \
	    --sysconfdir=/etc \
	)
	touch $(LIBIMECORE_DIR)/.configured

$(LIBIMECORE_DIR)/libimecore: $(LIBIMECORE_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(LIBIMECORE_DIR)
	touch -c $(LIBIMECORE_DIR)/libimecore

$(TARGET_DIR)/lib/libimecore: $(LIBIMECORE_DIR)/libimecore
	mkdir -p $(STAGING_DIR)/usr/include/imecore
	cp $(LIBIMECORE_DIR)/include/* $(STAGING_DIR)/usr/include/imecore
	cp -a $(LIBIMECORE_DIR)/.libs/libimecore.so* $(TARGET_DIR)/usr/lib
	cp -a $(LIBIMECORE_DIR)/.libs/libimecore.so* $(STAGING_DIR)/usr/lib
	touch -c $(TARGET_DIR)/bin/manager

libimecore: $(TARGET_DIR)/lib/libimecore

libimecore-source: $(DL_DIR)/$(LIBIMECORE_SOURCE)

libimecore-clean:
	-$(MAKE) -C $(LIBIMECORE_DIR) clean

libimecore-dirclean:
	rm -rf $(LIBIMECORE_DIR)


#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_LIBIMECORE),y)
TARGETS+=libimecore
endif
