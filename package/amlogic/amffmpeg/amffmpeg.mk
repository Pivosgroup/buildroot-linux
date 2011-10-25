#############################################################
#
# amffmpeg
#
#############################################################
AMFFMPEG_VERSION:=0.9.9
AMFFMPEG_SOURCE=amffmpeg-$(AMFFMPEG_VERSION).tar.gz
AMFFMPEG_SITE=./package/amlogic/amffmpeg/src
AMFFMPEG_SITE_METHOD=cp
AMFFMPEG_INSTALL_STAGING=YES

AMFFMPEG_CONF_OPT=--disable-static --enable-shared --disable-ffplay --disable-ffserver --disable-doc \
    --disable-mpegaudio-hp --disable-encoders --disable-decoder=h264 --disable-muxers --disable-filters \
    --disable-altivec --disable-amd3dnow \
    --disable-amd3dnowext --disable-mmx --disable-mmx2 --disable-sse --disable-ssse3 --disable-armv5te --disable-armv6 \
    --disable-armv6t2 --disable-armvfp --disable-iwmmxt --disable-mmi --disable-vis --disable-yasm --enable-pic --enable-zlib

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
		$(AMFFMPEG_CONF_OPT) \
        )
endef


ifdef DEBUG
AMFFMPEG_CONF_OPT+=    --enable-debug --disable-stripping
endif  

$(eval $(call AUTOTARGETS,package/amlogic,amffmpeg))
