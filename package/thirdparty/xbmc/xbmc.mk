################################################################################
#
## xbmc
#
#################################################################################

XBMC_VERSION = 7329e9ea3b9dbca6ff995be6fd0299783aa8be09
XBMC_SITE_METHOD = git
XBMC_SITE = ssh://git@github.com/Pivosgroup/xbmc.git
XBMC_INSTALL_STAGING = YES
XBMC_INSTALL_TARGET = YES

XBMC_DEPENDENCIES = host-lzo host-sdl_image

XBMC_CONF_OPT+= --enable-neon --enable-gles --disable-sdl --disable-x11 --disable-xrandr \
  --disable-projectm --enable-debug --disable-joystick --with-cpu=cortex-a9 \
  --enable-codec=amcodec --enable-player=amlplayer

XBMC_DEPENDENCIES += libogg flac libmad libmpeg2 libogg \
  libsamplerate libtheora libvorbis wavpack bzip2 dbus libcdio \
  python lzo zlib libgcrypt openssl mysql_client sqlite fontconfig \
  freetype jasper jpeg libmodplug libpng libungif tiff libcurl \
  libmicrohttpd libssh2 boost fribidi ncurses pcre libnfs afpfs-ng libplist libshairport libbluray \
  readline expat libxml2 yajl samba30 libass opengl libusb-compat avahi udev tinyxml taglib18 libssh

ifeq ($(BR2_PACKAGE_LIBAMPLAYERM1),y)
XBMC_DEPENDENCIES += libamplayerm1
endif

ifeq ($(BR2_PACKAGE_LIBAMPLAYERM3),y)
XBMC_DEPENDENCIES += libamplayerm3
endif

XBMC_CONF_ENV += PYTHON_VERSION="$(PYTHON_VERSION_MAJOR)"
XBMC_CONF_ENV += PYTHON_LDFLAGS="-L$(STAGING_DIR)/usr/lib/ -lpython$(PYTHON_VERSION_MAJOR) -lpthread -ldl -lutil -lm"
XBMC_CONF_ENV += PYTHON_CPPFLAGS="-I$(STAGING_DIR)/usr/include/python$(PYTHON_VERSION_MAJOR)"
XBMC_CONF_ENV += PYTHON_SITE_PKG="$(STAGING_DIR)/usr/lib/python$(PYTHON_VERSION_MAJOR)/site-packages"
XBMC_CONF_ENV += PYTHON_NOVERSIONCHECK="no-check"
XBMC_CONF_ENV += USE_TEXTUREPACKER_NATIVE_ROOT="$(HOST_DIR)/usr"

# For braindead apps like mysql that require running a binary/script
XBMC_CONF_ENV += PATH=$(STAGING_DIR)/usr/bin:$(TARGET_PATH)

define XBMC_BOOTSTRAP
  cd $(XBMC_DIR) && ./bootstrap
endef

define XBMC_INSTALL_ETC
  cp -rf package/thirdparty/xbmc/etc $(TARGET_DIR)
  cp -f package/thirdparty/xbmc/guisettings.xml $(TARGET_DIR)/usr/share/xbmc/system/
  cp -f package/thirdparty/xbmc/advancedsettings.xml $(TARGET_DIR)/usr/share/xbmc/system/
  cp -f package/thirdparty/xbmc/nobs.xml $(TARGET_DIR)/usr/share/xbmc/system/keymaps/

endef

define XBMC_INSTALL_SPLASH
  cp -f package/thirdparty/xbmc/xios_splash.png $(TARGET_DIR)/usr/share/xbmc/media/Splash.png
endef

define XBMC_CLEAN_UNUSED_ADDONS
  rm -rf $(TARGET_DIR)/usr/share/xbmc/addons/screensaver.rsxs.plasma
  rm -rf $(TARGET_DIR)/usr/share/xbmc/addons/visualization.milkdrop
  rm -rf $(TARGET_DIR)/usr/share/xbmc/addons/visualization.projectm
  rm -rf $(TARGET_DIR)/usr/share/xbmc/addons/visualization.itunes
endef

define XBMC_CLEAN_CONFLUENCE_SKIN
  find $(TARGET_DIR)/usr/share/xbmc/addons/skin.confluence/media -name *.png -delete
  find $(TARGET_DIR)/usr/share/xbmc/addons/skin.confluence/media -name *.jpg -delete
endef

define XBMC_STRIP_BINARIES
  find $(TARGET_DIR)/usr/lib/xbmc/ -name "*.so" -exec $(STRIPCMD) $(STRIP_STRIP_UNNEEDED) {} \;
  $(STRIPCMD) $(STRIP_STRIP_UNNEEDED) $(TARGET_DIR)/usr/lib/xbmc/xbmc.bin
endef

XBMC_PRE_CONFIGURE_HOOKS += XBMC_BOOTSTRAP
XBMC_POST_INSTALL_TARGET_HOOKS += XBMC_INSTALL_ETC
XBMC_POST_INSTALL_TARGET_HOOKS += XBMC_INSTALL_SPLASH
XBMC_POST_INSTALL_TARGET_HOOKS += XBMC_CLEAN_UNUSED_ADDONS
XBMC_POST_INSTALL_TARGET_HOOKS += XBMC_CLEAN_CONFLUENCE_SKIN
ifneq ($(BR2_ENABLE_DEBUG),y)
XBMC_POST_INSTALL_TARGET_HOOKS += XBMC_STRIP_BINARIES
endif

$(eval $(call AUTOTARGETS,package/thirdparty,xbmc))
