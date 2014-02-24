#############################################################
#
# mpd
#
#############################################################

MPD_VERSION = 0.16.8
MPD_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/project/musicpd/mpd/$(MPD_VERSION)
MPD_DEPENDENCIES = host-pkg-config libglib2

# Some options need an explicit --disable or --enable

ifeq ($(BR2_PACKAGE_MPD_ALSA),y)
MPD_DEPENDENCIES += alsa-lib
else
MPD_CONF_OPT += --disable-alsa
endif

ifeq ($(BR2_PACKAGE_MPD_AO),y)
MPD_DEPENDENCIES += libao
MPD_CONF_OPT += --enable-ao
endif

ifeq ($(BR2_PACKAGE_MPD_AUDIOFILE),y)
MPD_DEPENDENCIES += audiofile
MPD_CONF_OPT += --enable-audiofile
endif

ifeq ($(BR2_PACKAGE_MPD_PULSEAUDIO),y)
MPD_DEPENDENCIES += pulseaudio
MPD_CONF_OPT += --enable-pulse
endif

ifeq ($(BR2_PACKAGE_MPD_BZIP2),y)
MPD_DEPENDENCIES += bzip2
MPD_CONF_OPT += --enable-bzip2
endif

ifeq ($(BR2_PACKAGE_MPD_FAAD2),y)
MPD_DEPENDENCIES += faad2
else
MPD_CONF_OPT += --disable-faad2
endif

ifeq ($(BR2_PACKAGE_MPD_FLAC),y)
MPD_DEPENDENCIES += flac
else
MPD_CONF_OPT += --without-flac --disable-oggflac
endif

ifeq ($(BR2_PACKAGE_MPD_CURL),y)
MPD_DEPENDENCIES += libcurl
else
MPD_CONF_OPT += --disable-curl
endif

ifeq ($(BR2_PACKAGE_MPD_LAME),y)
MPD_DEPENDENCIES += lame
else
MPD_CONF_OPT += --disable-lame-encoder
endif

ifeq ($(BR2_PACKAGE_MPD_LIBCUE),y)
MPD_DEPENDENCIES += libcue
else
MPD_CONF_OPT += --disable-cue
endif

ifeq ($(BR2_PACKAGE_MPD_LIBSAMPLERATE),y)
MPD_DEPENDENCIES += libsamplerate
else
MPD_CONF_OPT += --disable-lsr
endif

ifeq ($(BR2_PACKAGE_MPD_LIBSNDFILE),y)
MPD_DEPENDENCIES += libsndfile
else
MPD_CONF_OPT += --disable-sndfile
endif

ifeq ($(BR2_PACKAGE_MPD_VORBIS),y)
MPD_DEPENDENCIES += libvorbis
else
MPD_CONF_OPT += --disable-vorbis
endif

ifeq ($(BR2_PACKAGE_MPD_MPG123),y)
MPD_DEPENDENCIES += libid3tag mpg123
else
MPD_CONF_OPT += --disable-mpg123
endif

ifeq ($(BR2_PACKAGE_MPD_MUSEPACK),y)
MPD_DEPENDENCIES += musepack
else
MPD_CONF_OPT += --disable-mpc
endif

ifeq ($(BR2_PACKAGE_MPD_SQLITE),y)
MPD_DEPENDENCIES += sqlite
else
MPD_CONF_OPT += --disable-sqlite
endif

ifneq ($(BR2_PACKAGE_MPD_TCP),y)
MPD_CONF_OPT += --disable-tcp
endif

ifeq ($(BR2_PACKAGE_MPD_TREMOR),y)
MPD_DEPENDENCIES += tremor
MPD_CONF_OPT += --with-tremor
endif

ifeq ($(BR2_PACKAGE_MPD_WAVPACK),y)
MPD_DEPENDENCIES += wavpack
else
MPD_CONF_OPT += --disable-wavpack
endif

ifeq ($(BR2_PACKAGE_MPD_FFMPEG),y)
MPD_DEPENDENCIES += ffmpeg
MPD_CONF_OPT += --enable-ffmpeg
else
MPD_CONF_OPT += --disable-ffmpeg
endif

define MPD_INSTALL_EXTRA_FILES
	@if [ ! -f $(TARGET_DIR)/etc/mpd.conf ]; then \
		$(INSTALL) -D package/multimedia/mpd/mpd.conf \
			$(TARGET_DIR)/etc/mpd.conf; \
	fi
	$(INSTALL) -m 0755 -D package/multimedia/mpd/S95mpd \
		$(TARGET_DIR)/etc/init.d/S95mpd
endef

MPD_POST_INSTALL_TARGET_HOOKS += MPD_INSTALL_EXTRA_FILES

$(eval $(call AUTOTARGETS))
