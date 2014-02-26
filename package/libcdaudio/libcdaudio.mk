################################################################################
#
# libcdaudio
#
################################################################################

LIBCDAUDIO_VERSION = 0.99.12p2
LIBCDAUDIO_SITE = http://downloads.sourceforge.net/project/libcdaudio/libcdaudio/$(LIBCDAUDIO_VERSION)/
LIBCDAUDIO_INSTALL_STAGING = YES
LIBCDAUDIO_CONFIG_SCRIPTS = libcdaudio-config

$(eval $(autotools-package))
