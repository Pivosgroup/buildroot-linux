#############################################################
#
# mplayer
#
#############################################################
MPLAYER_VERSION = 32726
# MPLAYER_SOURCE = MPlayer-$(MPLAYER_VERSION).tar.bz2
# MPLAYER_SITE = http://www.mplayerhq.hu/MPlayer/releases
MPLAYER_SITE = svn://svn.mplayerhq.hu/mplayer/trunk

MPLAYER_CFLAGS = $(TARGET_CFLAGS)
MPLAYER_LDFLAGS = $(TARGET_LDFLAGS)

# mplayer needs pcm+mixer support, but configure fails to check for it
ifeq ($(BR2_PACKAGE_ALSA_LIB)$(BR2_PACKAGE_ALSA_LIB_MIXER)$(BR2_PACKAGE_ALSA_LIB_PCM),yyy)
MPLAYER_DEPENDENCIES += alsa-lib
MPLAYER_CONF_OPTS    += --enable-alsa
else
MPLAYER_CONF_OPTS    += --disable-alsa
endif

ifeq ($(BR2_ENDIAN),"BIG")
MPLAYER_CONF_OPTS += --enable-big-endian
else
MPLAYER_CONF_OPTS += --disable-big-endian
endif

# mplayer unfortunately uses --disable-largefiles, so we cannot use
# DISABLE_LARGEFILE
ifeq ($(BR2_LARGEFILE),y)
MPLAYER_CONF_OPTS += --enable-largefiles
else
# dvdread/dvdcss requires largefile support
MPLAYER_CONF_OPTS += 			\
	--disable-largefiles 		\
	--disable-dvdread-internal 	\
	--disable-libdvdcss-internal
endif

ifeq ($(BR2_PACKAGE_SDL),y)
MPLAYER_CONF_OPTS += \
	--enable-sdl \
	--with-sdl-config=$(STAGING_DIR)/usr/bin/sdl-config
MPLAYER_DEPENDENCIES += sdl
else
MPLAYER_CONF_OPTS += --disable-sdl
endif

ifeq ($(BR2_PACKAGE_FREETYPE),y)
MPLAYER_CONF_OPTS +=  \
	--enable-freetype \
	--with-freetype-config=$(STAGING_DIR)/usr/bin/freetype-config
MPLAYER_DEPENDENCIES += freetype
else
MPLAYER_CONF_OPTS += --disable-freetype
endif

ifeq ($(BR2_PACKAGE_MPLAYER_MPLAYER),y)
MPLAYER_CONF_OPTS += --enable-mplayer
else
MPLAYER_CONF_OPTS += --disable-mplayer
endif

ifeq ($(BR2_PACKAGE_MPLAYER_MENCODER),y)
MPLAYER_CONF_OPTS += --enable-mencoder
else
MPLAYER_CONF_OPTS += --disable-mencoder
endif

ifeq ($(BR2_PACKAGE_TREMOR),y)
MPLAYER_DEPENDENCIES += tremor
MPLAYER_CONF_OPTS += --disable-tremor-internal --enable-tremor
endif

ifeq ($(BR2_PACKAGE_LIBVORBIS),y)
MPLAYER_DEPENDENCIES += libvorbis
MPLAYER_CONF_OPTS += --enable-libvorbis
endif

ifeq ($(BR2_PACKAGE_LIBMAD),y)
MPLAYER_DEPENDENCIES += libmad
else
MPLAYER_CONF_OPTS += --disable-mad
endif

ifeq ($(BR2_PACKAGE_LIVE555),y)
MPLAYER_DEPENDENCIES += live555
MPLAYER_CONF_OPTS += --enable-live
MPLAYER_LIVE555 = liveMedia groupsock UsageEnvironment BasicUsageEnvironment
MPLAYER_CFLAGS += \
	$(addprefix -I$(STAGING_DIR)/usr/include/live/,$(MPLAYER_LIVE555))
MPLAYER_LDFLAGS += $(addprefix -l,$(MPLAYER_LIVE555)) -lstdc++
else
MPLAYER_CONF_OPTS += --disable-live
endif

MPLAYER_DEPENDENCIES += $(if $(BR2_PACKAGE_LIBTHEORA),libtheora)
MPLAYER_DEPENDENCIES += $(if $(BR2_PACKAGE_LIBPNG),libpng)
MPLAYER_DEPENDENCIES += $(if $(BR2_PACKAGE_JPEG),jpeg)
MPLAYER_DEPENDENCIES += $(if $(BR2_PACKAGE_XLIB_LIBX11),xlib_libX11)
MPLAYER_DEPENDENCIES += $(if $(BR2_PACKAGE_XLIB_LIBXV),xlib_libXv)

# ARM optimizations
ifeq ($(call qstrip,$(BR2_GCC_TARGET_ARCH)),armv5te)
MPLAYER_CONF_OPTS += --enable-armv5te
endif

ifeq ($(call qstrip,$(BR2_GCC_TARGET_ARCH)),armv6j)
MPLAYER_CONF_OPTS += --enable-armv6
endif

ifeq ($(call qstrip,$(BR2_GCC_TARGET_ARCH)),armv7-a)
MPLAYER_CONF_OPTS += --enable-neon
endif

ifeq ($(BR2_i386),y)
# inline asm breaks with "can't find a register in class 'GENERAL_REGS'"
# inless we free up ebp
MPLAYER_CFLAGS += -fomit-frame-pointer
endif

define MPLAYER_CONFIGURE_CMDS
	(cd $(@D); rm -rf config.cache; \
		$(TARGET_CONFIGURE_OPTS) \
		$(TARGET_CONFIGURE_ARGS) \
		./configure \
		--prefix=/usr \
		--confdir=/etc \
		--target=$(GNU_TARGET_NAME) \
		--host-cc="$(HOSTCC)" \
		--cc="$(TARGET_CC)" \
		--as="$(TARGET_AS)" \
		--charset=UTF-8 \
		--extra-cflags="$(MPLAYER_CFLAGS)" \
		--extra-ldflags="$(MPLAYER_LDFLAGS)" \
		--yasm='' \
		--enable-mad \
		--enable-fbdev \
		$(MPLAYER_CONF_OPTS) \
		--enable-cross-compile \
		--disable-ivtv \
		--enable-dynamic-plugins \
	)
endef

# this is available on uClibc 0.9.31 even without ipv6 support, breaking the
# build in ffmpeg/libavformat/udp.c
ifneq ($(BR2_INET_IPV6),y)
define MPLAYER_FIXUP_IPV6_MREQ_DETECTION
	$(SED) 's/\(#define HAVE_STRUCT_IPV6_MREQ\) 1/\1 0/' $(@D)/config.h
endef

MPLAYER_POST_CONFIGURE_HOOKS += MPLAYER_FIXUP_IPV6_MREQ_DETECTION
endif

define MPLAYER_BUILD_CMDS
	$(MAKE) -C $(@D)
endef

define MPLAYER_INSTALL_TARGET_CMDS
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(@D) install
endef

define MPLAYER_UNINSTALL_TARGET_CMDS
	$(MAKE) DESTDIR=$(TARGET_DIR) -C $(@D) uninstall
endef

define MPLAYER_CLEAN_CMDS
	$(MAKE) -C $(@D) clean
endef

$(eval $(call GENTARGETS))
