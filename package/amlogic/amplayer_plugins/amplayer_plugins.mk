#############################################################
#
# amplayer_plugins
#
#############################################################
AMPLAYER_PLUGINS_VERSION:=0.9.9
AMPLAYER_PLUGINS_DIR=$(BUILD_DIR)/amplayer_plugins
AMPLAYER_PLUGINS_SOURCE=src
AMPLAYER_PLUGINS_SITE=.

export PREFIX=$(TARGET_DIR)/usr
export STAGING=$(STAGING_DIR)/usr

$(AMPLAYER_PLUGINS_DIR)/.unpacked:amplayer_plugins-unpacked

amplayer_plugins-unpacked:
	mkdir -p $(AMPLAYER_PLUGINS_DIR)
	cp -arf ./package/multimedia/amplayer_plugins/src/* $(AMPLAYER_PLUGINS_DIR)
	touch $(AMPLAYER_PLUGINS_DIR)/.unpacked

$(AMPLAYER_PLUGINS_DIR)/.installed:$(AMPLAYER_PLUGINS_DIR)/amplayer_plugins

$(AMPLAYER_PLUGINS_DIR)/amplayer_plugins: $(AMPLAYER_PLUGINS_DIR)/.unpacked
	@if [ ! -d "$(STAGING_DIR)/usr/include/player_plugins" ]; then \
		mkdir -p $(STAGING_DIR)/usr/include/player_plugins;	\
	fi;
	#make CC=$(TARGET_CC) LINK=$(TARGET_LD) -C $(AMPLAYER_PLUGINS_DIR)/src dist
	make CC=$(TARGET_CC) LINK=$(TARGET_LD) -C $(AMPLAYER_PLUGINS_DIR)/src all
	make CC=$(TARGET_CC) LINK=$(TARGET_LD) -C $(AMPLAYER_PLUGINS_DIR)/testcase/benchmark
	install -m 555 $(AMPLAYER_PLUGINS_DIR)/testcase/bin/mp_tools $(PREFIX)/bin 
		
	install -m 755 $(AMPLAYER_PLUGINS_DIR)/lib/*.so $(PREFIX)/lib	
	install -m 644 $(AMPLAYER_PLUGINS_DIR)/include/mp_api.h  $(STAGING_DIR)/usr/include/player_plugins
	install -m 644 $(AMPLAYER_PLUGINS_DIR)/include/mp_types.h  $(STAGING_DIR)/usr/include/player_plugins
	install -m 644 $(AMPLAYER_PLUGINS_DIR)/include/mp_utils.h  $(STAGING_DIR)/usr/include/player_plugins
	install -m 644 $(AMPLAYER_PLUGINS_DIR)/include/msgmngr.h  $(STAGING_DIR)/usr/include/player_plugins
	install -m 644 $(AMPLAYER_PLUGINS_DIR)/include/mp_events.h  $(STAGING_DIR)/usr/include/player_plugins
	#make amplayer_plugins_test
	touch $(AMPLAYER_PLUGINS_DIR)/.installed

amplayer_plugins: $(AMPLAYER_PLUGINS_DIR)/.installed

amplayer_plugins-source: $(DL_DIR)/$(AMPLAYER_PLUGINS_SOURCE)

amplayer_plugins-clean:
	-$(MAKE) -C $(AMPLAYER_PLUGINS_DIR) clean

amplayer_plugins-dirclean:
	rm -rf $(AMPLAYER_PLUGINS_DIR)


amplayer_plugins_test:	
	#make CC=$(TARGET_CC) LINK=$(TARGET_LD) -C $(AMPLAYER_PLUGINS_DIR)/testcase/benchmark
	make CC=$(TARGET_CC) LINK=$(TARGET_LD) -C $(AMPLAYER_PLUGINS_DIR)/testcase/benchmark
	install -m 555 $(AMPLAYER_PLUGINS_DIR)/testcase/bin/mp_tools $(PREFIX)/bin 	

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_AMPLAYER_PLUGINS),y)
TARGETS+=amplayer_plugins
endif
