AMFFMPEG_CONF_OPT= --disable-static --enable-shared \
                --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-doc \
                --disable-encoders --disable-muxers --disable-altivec \
                --disable-amd3dnow --disable-amd3dnowext --disable-mmx --disable-mmx2 --disable-sse --disable-ssse3 \
                --disable-armv5te --disable-armv6t2 --disable-iwmmxt --disable-mmi --disable-vis --disable-yasm \
                --enable-cross-compile --arch=arm --cpu=cortex-a9 --enable-neon --target-os=linux \
                --enable-pthreads --enable-runtime-cpudetect --enable-pic --enable-avfilter --enable-postproc --enable-gpl \
                --enable-librtmp --pkg-config=pkg-config

define AMFFMPEG_CONFIGURE_CMDS
	(cd $(AMFFMPEG_DIR) && rm -rf config.cache && \
	$(TARGET_CONFIGURE_OPTS) \
	$(TARGET_CONFIGURE_ARGS) \
	$(AMFFMPEG_CONF_ENV) \
	./configure \
		--enable-cross-compile  \
		--cross-prefix=$(TARGET_CROSS) \
		--sysroot=$(STAGING_DIR) \
		--host-cc="$(HOSTCC)" \
		--arch=$(BR2_ARCH) \
		--prefix=/usr \
		--extra-cflags="-mfloat-abi=softfp -mfpu=neon -march=armv7-a" \
		$(AMFFMPEG_CONF_OPT) \
        )
endef

define AMFFMPEG_BUILD_CMDS
	make -C $(AMFFMPEG_DIR)
endef

ifdef DEBUG
AMFFMPEG_CONF_OPT+=    --enable-debug --disable-stripping
endif  

define AMFFMPEG_INSTALL_STAGING_CMDS
	make -C $(AMFFMPEG_DIR) install DESTDIR="$(STAGING_DIR)"
	install $(AMFFMPEG_DIR)/libavformat/aviolpbuf.h $(STAGING_DIR)/usr/include/libavformat/
endef

define AMFFMPEG_INSTALL_TARGET_CMDS
	make -C $(AMFFMPEG_DIR) DESTDIR="$(TARGET_DIR)"
endef
