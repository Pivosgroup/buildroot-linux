#############################################################
#
# kplayer
#
#############################################################
KPLAYER_VERSION:=0.9.9
KPLAYER_DIR=$(BUILD_DIR)/kplayer
KPLAYER_SOURCE=src
KPLAYER_SITE=.
KPLAYER_DEPENDENCIES=libplayer

$(KPLAYER_DIR)/.unpacked:


kplayer-unpack:
	mkdir -p $(KPLAYER_DIR)
	cp -arf ./package/multimedia/kplayer/src/* $(KPLAYER_DIR)
	touch $(KPLAYER_DIR)/.unpacked

$(KPLAYER_DIR)/kplayer: $(KPLAYER_DIR)/.unpacked
	$(MAKE) CC=$(TARGET_CC) -C $(KPLAYER_DIR)
	#touch -c $(KPLAYER_DIR)/kplayer

$(TARGET_DIR)/bin/kplayer: $(KPLAYER_DIR)/kplayer
	install -m 755 $(KPLAYER_DIR)/kplayer $(TARGET_DIR)/bin
	#touch -c $(TARGET_DIR)/bin/kplayer

kplayer:libplayer kplayer-unpack $(TARGET_DIR)/bin/kplayer

kplayer-source: $(DL_DIR)/$(KPLAYER_SOURCE)

kplayer-clean:
	-$(MAKE) -C $(KPLAYER_DIR) clean

kplayer-dirclean:
	rm -rf $(KPLAYER_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_KPLAYER),y)
TARGETS+=kplayer
endif
