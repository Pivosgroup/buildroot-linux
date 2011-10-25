#############################################################
#
# amplayer2
#
#############################################################
AMPLAYER2_VERSION:=0.9.9
AMPLAYER2_DIR=$(BUILD_DIR)/amplayer2
AMPLAYER2_SOURCE=${AMPLAYER2-D}
AMPLAYER2_SITE=.

export PREFIX=$(TARGET_DIR)/usr
export STAGING=$(STAGING_DIR)/usr

AMPLAYER2_DEPENDENCIES=zlib
AMPLAYER2_TARGET_BINARY=$(PREFIX)/usr/bin/amplayer2


$(AMPLAYER2_DIR)/.unpacked:
	-rm -rf $(AMPLAYER2_DIR)
	mkdir -p $(AMPLAYER2_DIR)
	cp -arf ./package/multimedia/amplayer2/src/* $(AMPLAYER2_DIR)
	./package/multimedia/amplayer2/get_svn_info.sh $(AMPLAYER2_DIR)/control/include/version.h
	touch $@

$(AMPLAYER2_TARGET_BINARY): $(AMPLAYER2_DIR)/.unpacked
	$(MAKE) CC=$(TARGET_CC) -C $(AMPLAYER2_DIR) install

amplayer2: $(AMPLAYER2_TARGET_BINARY)

amplayer2-clean:
	-$(MAKE) -C $(AMPLAYER2_DIR) clean

amplayer2-dirclean:
	rm -rf $(AMPLAYER2_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_AMPLAYER2),y)
TARGETS+=amplayer2
endif
