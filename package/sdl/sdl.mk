################################################################################
#
# sdl
#
################################################################################

SDL_VERSION = 1.2.15
SDL_SOURCE = SDL-$(SDL_VERSION).tar.gz
SDL_SITE = http://www.libsdl.org/release
SDL_LICENSE = LGPLv2.1+
SDL_LICENSE_FILES = COPYING
SDL_INSTALL_STAGING = YES

ifeq ($(BR2_PACKAGE_SDL_FBCON),y)
SDL_CONF_OPT += --enable-video-fbcon=yes
else
SDL_CONF_OPT += --enable-video-fbcon=no
endif

ifeq ($(BR2_PACKAGE_SDL_DIRECTFB),y)
SDL_DEPENDENCIES += directfb
SDL_CONF_OPT += --enable-video-directfb=yes
SDL_CONF_ENV = ac_cv_path_DIRECTFBCONFIG=$(STAGING_DIR)/usr/bin/directfb-config
else
SDL_CONF_OPT = --enable-video-directfb=no
endif

ifeq ($(BR2_PACKAGE_SDL_QTOPIA),y)
SDL_CONF_OPT += --enable-video-qtopia=yes
SDL_DEPENDENCIES += qt
else
SDL_CONF_OPT += --enable-video-qtopia=no
endif

ifeq ($(BR2_PACKAGE_SDL_X11),y)
SDL_CONF_OPT += --enable-video-x11=yes
SDL_DEPENDENCIES += xlib_libX11 xlib_libXext \
	$(if $(BR2_PACKAGE_XLIB_LIBXRENDER), xlib_libXrender) \
	$(if $(BR2_PACKAGE_XLIB_LIBXRANDR), xlib_libXrandr)
else
SDL_CONF_OPT += --enable-video-x11=no
endif

ifeq ($(BR2_PACKAGE_TSLIB),y)
SDL_DEPENDENCIES += tslib
endif

ifeq ($(BR2_PACKAGE_ALSA_LIB),y)
SDL_DEPENDENCIES += alsa-lib
endif

ifeq ($(BR2_PACKAGE_MESA3D),y)
SDL_DEPENDENCIES += mesa3d
endif

SDL_CONF_OPT += --enable-pulseaudio=no \
		--disable-arts \
		--disable-esd \
		--disable-nasm \
		--disable-video-ps3

define HOST_SDL_AUTOGEN
	cd $(@D) && ./autogen.sh
endef

HOST_SDL_PRE_CONFIGURE_HOOKS += HOST_SDL_AUTOGEN



SDL_CONFIG_SCRIPTS = sdl-config

# Remove the -Wl,-rpath option.
define SDL_FIXUP_SDL_CONFIG
	$(SED) 's%-Wl,-rpath,\$${libdir}%%' \
		$(STAGING_DIR)/usr/bin/sdl-config
endef

SDL_POST_INSTALL_STAGING_HOOKS += SDL_FIXUP_SDL_CONFIG

$(eval $(autotools-package))
$(eval $(host-autotools-package))