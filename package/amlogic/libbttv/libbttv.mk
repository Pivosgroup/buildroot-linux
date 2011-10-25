#############################################################
#
# libbttv
#
#############################################################
LIBBTTV_VERSION:=1.0.0
LIBBTTV_SOURCE:=libbttv-$(LIBBTTV-VERSION).tar.gz
LIBBTTV_SITE:=.
LIBBTTV_DIR:=$(BUILD_DIR)/libbttv-$(LIBBTTV_VERSION)
LIBBTTV_BINARY:=libbttv
LIBBTTV_TARGET_BINARY:=usr/bin/libbttv



#CXXFLAGS=-I$(BUILD_DIR)/qt-everywhere-opensource-src-4.6.2/include

$(LIBBTTV_DIR)/.unpacked:
	mkdir -p $(LIBBTTV_DIR)
	cp -arf ./package/libbttv/src/* $(LIBBTTV_DIR)
	touch $(LIBBTTV_DIR)/.unpacked

$(LIBBTTV_DIR)/.configured: $(LIBBTTV_DIR)/.unpacked
	(cd $(LIBBTTV_DIR); rm -rf config.cache; \
	    LDFLAGS="-L$(TARGET_DIR)/usr/lib" \
	    ./configure \
	    --target=$(GNU_TARGET_NAME) \
	    --host=$(GNU_TARGET_NAME) \
	    --build=$(GNU_HOST_NAME) \
	    --prefix=/usr \
	    --sysconfdir=/etc \
	)
	touch $(LIBBTTV_DIR)/.configured

$(LIBBTTV_DIR)/$(LIBBTTV_BINARY): $(LIBBTTV_DIR)/.configured
	$(MAKE) CC=$(TARGET_CC) -C $(LIBBTTV_DIR) 
	touch -c $(LIBBTTV_DIR)/libbttv
	
$(TARGET_DIR)/$(LIBBTTV_TARGET_BINARY): $(LIBBTTV_DIR)/$(LIBBTTV_BINARY)
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(LIBBTTV_DIR) install-strip
	rm -Rf $(TARGET_DIR)/usr/man

libbttv: libxfer opengl qt $(TARGET_DIR)/$(LIBBTTV_TARGET_BINARY)

libbttv-source: $(DL_DIR)/$(LIBBTTV_SOURCE)

libbttv-clean:
	$(MAKE) prefix=$(TARGET_DIR)/usr -C $(LIBBTTV_DIR) uninstall
	-$(MAKE) -C $(LIBBTTV_DIR) clean


libbttv-dirclean:
	rm -rf $(LIBBTTV_DIR)


#############################################################
#
# Toplevel Makefile options
#
#############################################################
#ifeq ($(BR2_PACKAGE_LIBBTTV),y)
#TARGETS+=libbttv
#endif
