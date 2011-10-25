#############################################################
#
# libxfer
#
#############################################################
LIBXFER_VERSION:=1.1.0
LIBXFER_DIR=$(BUILD_DIR)/libxfer-$(LIBXFER_VERSION)
LIBXFER_SOURCE=src
LIBXFER_SITE=.
LIBXFER_DEPENDENCIES= zlib libcurl

$(LIBXFER_DIR)/.unpacked:
	mkdir -p $(LIBXFER_DIR)
	cp -arf ./package/libxfer/src/* $(LIBXFER_DIR)
	touch $(LIBXFER_DIR)/.unpacked

$(LIBXFER_DIR)/.configured: $(LIBXFER_DIR)/.unpacked
	(cd $(LIBXFER_DIR); rm -rf config.cache; \
	    $(TARGET_CONFIGURE_OPTS) \
	    $(TARGET_CONFIGURE_ARGS) \
	    LDFLAGS="-L$(STAGING_DIR)/usr/lib -L/opt/CodeSourcery/Sourcery_G++_Lite/arm-none-linux-gnueabi/libc/thumb2/" \
	    ./configure \
	    --target=$(GNU_TARGET_NAME) \
	    --host=$(GNU_TARGET_NAME) \
	    --build=$(GNU_HOST_NAME) \
	    --with-zlib=yes \
	    --enable-thunder=no \
	    --enable-debug=yes \
	    --enable-demo=no \
	    --prefix=/usr \
	    --sysconfdir=/etc \
	)
	touch $(LIBXFER_DIR)/.configured

$(LIBXFER_DIR)/libxfer: $(LIBXFER_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(LIBXFER_DIR)
	touch -c $(LIBXFER_DIR)/libxfer

$(TARGET_DIR)/lib/libxfer: $(LIBXFER_DIR)/libxfer
	mkdir -p $(STAGING_DIR)/usr/include/xfer
	cp $(LIBXFER_DIR)/include/* $(STAGING_DIR)/usr/include/xfer
	cp -a $(LIBXFER_DIR)/src/.libs/libxfer.so* $(TARGET_DIR)/usr/lib
	cp -a $(LIBXFER_DIR)/src/.libs/libxfer.so* $(STAGING_DIR)/usr/lib
	touch -c $(TARGET_DIR)/bin/manager

libxfer: zlib opengl libcurl $(TARGET_DIR)/lib/libxfer

libxfer-source: $(DL_DIR)/$(LIBXFER_SOURCE)

libxfer-clean:
	-$(MAKE) -C $(LIBXFER_DIR) clean

libxfer-dirclean:
	rm -rf $(LIBXFER_DIR)


#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_LIBXFER),y)
TARGETS+=libxfer
endif
