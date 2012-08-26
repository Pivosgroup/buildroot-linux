#############################################################
#
# amffmpeg
#
#############################################################
AMFFMPEG_VERSION:=0.9.9
AMFFMPEG_SOURCE=amffmpeg-$(AMFFMPEG_VERSION).tar.gz
AMFFMPEG_SITE=./package/amlogic/libplayer/src/amffmpeg
AMFFMPEG_SITE_METHOD=cp
AMFFMPEG_INSTALL_STAGING=YES
AMFFMPEG_DEPENDENCIES = alsa-lib librtmp

AMFFMPEG_CONF_OPT= --disable-static --enable-shared \
                --disable-ffmpeg --disable-ffplay --disable-ffserver --disable-doc \
                --disable-encoders --disable-muxers --disable-altivec \
                --disable-amd3dnow --disable-amd3dnowext --disable-mmx --disable-mmx2 --disable-sse --disable-ssse3 \
                --disable-armv5te --disable-armv6t2 --disable-iwmmxt --disable-mmi --disable-vis --disable-yasm \
                --enable-cross-compile --arch=arm --cpu=cortex-a9 --enable-neon --target-os=linux \
                --enable-pthreads --enable-runtime-cpudetect --enable-pic --enable-avfilter --enable-postproc --enable-gpl \
                --enable-librtmp --pkg-config=pkg-config

define AMFFMPEG_CONFIGURE_CMDS
	(cd $(AMFFMPEG_SRCDIR) && rm -rf config.cache && \
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


ifdef DEBUG
AMFFMPEG_CONF_OPT+=    --enable-debug --disable-stripping
endif  

define AMFFMPEG_STAGING_AMFFMPEG_EXTRA_HEADERS
       install $(@D)/libavformat/aviolpbuf.h $(STAGING_DIR)/usr/include/libavformat
endef

AMFFMPEG_POST_INSTALL_STAGING_HOOKS += AMFFMPEG_STAGING_AMFFMPEG_EXTRA_HEADERS

$(eval $(call AUTOTARGETS,package/amlogic,amffmpeg))
