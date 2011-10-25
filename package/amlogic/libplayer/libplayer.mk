#############################################################
#
# libplayer
#
#############################################################
LIBPLAYER_VERSION:=0.9.9
LIBPLAYER_DIR=$(BUILD_DIR)/libplayer
#AMFFMPEG_DIR=$(LIBPLAYER_DIR)/amffmpeg
LIBPLAYER_SOURCE=src
LIBPLAYER_SITE=.

PREFIX=$(TARGET_DIR)
export INC_DIR=$(STAGING_DIR)/usr

FFMPEG_CONFIG_FLAGS=--arch=arm --enable-cross-compile \
		--cross-prefix=arm-none-linux-gnueabi- --prefix=${PREFIX} \
		--incdir=$(STAGING_DIR)/usr/include --extra-ldflags=-L$(PREFIX)/lib --disable-static --enable-shared \
		--disable-ffmpeg --disable-ffplay --disable-ffserver --disable-doc --disable-mpegaudio-hp \
		--disable-encoders --disable-decoder=h264 --disable-muxers --disable-filters --disable-altivec \
		--disable-amd3dnow --disable-amd3dnowext --disable-mmx --disable-mmx2 --disable-sse --disable-ssse3 \
		--disable-armv5te --disable-armv6 --disable-armv6t2 --disable-armvfp --disable-iwmmxt --disable-mmi --disable-vis --disable-yasm \
		--enable-pic

$(LIBPLAYER_DIR)/.unpacked:libplayer-unpacked

libplayer-unpacked:
	-rm -rf $(LIBPLAYER_DIR)
	mkdir -p $(LIBPLAYER_DIR)
	cp -arf ./package/multimedia/libplayer/src/* $(LIBPLAYER_DIR)
	touch $(LIBPLAYER_DIR)/.unpacked

$(LIBPLAYER_DIR)/.installed:$(LIBPLAYER_DIR)/libplayer

$(LIBPLAYER_DIR)/libplayer: $(LIBPLAYER_DIR)/.unpacked
	cd $(AMFFMPEG_DIR) && $(TOPDIR)/package/multimedia/libplayer/src/amffmpeg/configure ${FFMPEG_CONFIG_FLAGS}
	$(MAKE) CC=$(TARGET_CC) -C $(LIBPLAYER_DIR) install
	touch $(LIBPLAYER_DIR)/.installed

libplayer:$(LIBPLAYER_DIR)/.installed

libplayer-source: $(DL_DIR)/$(LIBPLAYER_SOURCE)

libplayer-clean:
	-$(MAKE) -C $(LIBPLAYER_DIR) clean

libplayer-dirclean:
	rm -rf $(LIBPLAYER_DIR)

#############################################################
#
# Toplevel Makefile options
#
#############################################################
ifeq ($(BR2_PACKAGE_LIBPLAYER),y)
TARGETS+=libplayer
endif
