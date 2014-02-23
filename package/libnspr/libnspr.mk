#############################################################
#
# libnspr
#
#############################################################
LIBNSPR_VERSION = 4.8.7
LIBNSPR_SOURCE = nspr-$(LIBNSPR_VERSION).tar.gz
LIBNSPR_SITE = https://ftp.mozilla.org/pub/mozilla.org/nspr/releases/v$(LIBNSPR_VERSION)/src/
LIBNSPR_SUBDIR = mozilla/nsprpub
LIBNSPR_INSTALL_STAGING = YES
# Set the host CFLAGS and LDFLAGS so NSPR does not guess wrongly
LIBNSPR_CONF_ENV = HOST_CFLAGS="-g -O2" \
		   HOST_LDFLAGS="-lc"
# NSPR mixes up --build and --host
LIBNSPR_CONF_OPT = --host=$(GNU_HOST_NAME)

$(eval $(call AUTOTARGETS))
