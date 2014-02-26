################################################################################
#
# libnspr
#
################################################################################

LIBNSPR_VERSION = 4.9.6
LIBNSPR_SOURCE = nspr-$(LIBNSPR_VERSION).tar.gz
LIBNSPR_SITE = https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v$(LIBNSPR_VERSION)/src/
LIBNSPR_SUBDIR = mozilla/nsprpub
LIBNSPR_INSTALL_STAGING = YES
LIBNSPR_CONFIG_SCRIPTS = nspr-config
LIBNSPR_LICENSE = MPLv2.0
LIBNSPR_LICENSE_FILES = mozilla/nsprpub/LICENSE

# Set the host CFLAGS and LDFLAGS so NSPR does not guess wrongly
LIBNSPR_CONF_ENV = HOST_CFLAGS="-g -O2" \
		   HOST_LDFLAGS="-lc"
# NSPR mixes up --build and --host
LIBNSPR_CONF_OPT  = --host=$(GNU_HOST_NAME)
LIBNSPR_CONF_OPT += --$(if $(BR2_ARCH_IS_64),en,dis)able-64bit
LIBNSPR_CONF_OPT += --$(if $(BR2_INET_IPV6),en,dis)able-ipv6

ifeq ($(BR2_arm),y)
ifeq ($(BR2_cortex_a8)$(BR2_cortex_a9),y)
LIBNSPR_CONF_OPT += --enable-thumb2
else
LIBNSPR_CONF_OPT += --disable-thumb2
endif
endif

$(eval $(autotools-package))
