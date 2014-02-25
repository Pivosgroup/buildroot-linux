#############################################################
#
# SDL_gfx addon for SDL
#
#############################################################
SDL_GFX_VERSION = 2.0.19
SDL_GFX_SOURCE = SDL_gfx-$(SDL_GFX_VERSION).tar.gz
SDL_GFX_SITE = http://www.ferzkopp.net/Software/SDL_gfx-2.0/
SDL_GFX_INSTALL_STAGING = YES
SDL_GFX_DEPENDENCIES = sdl
SDL_GFX_CONF_OPT = \
	--with-sdl-prefix=$(STAGING_DIR)/usr \
	--disable-sdltest \
	--enable-static

# Even though x86_64 processors support MMX, the MMX-specific assembly
# code in sdl_gfx is IA32 specific, and does not build for x86_64.
ifeq ($(BR2_i386)$(BR2_X86_CPU_HAS_MMX),yy)
SDL_GFX_CONF_OPT += --enable-mmx
else
SDL_GFX_CONF_OPT += --disable-mmx
endif

$(eval $(autotools-package))
