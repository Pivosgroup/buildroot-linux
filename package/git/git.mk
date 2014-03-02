################################################################################
#
# git
#
################################################################################

GIT_VERSION         = 1.8.4.1
GIT_SITE            = http://git-core.googlecode.com/files/
GIT_LICENSE         = GPLv2 LGPLv2.1+
GIT_LICENSE_FILES   = COPYING LGPL-2.1
GIT_DEPENDENCIES    = zlib host-gettext

ifeq ($(BR2_PACKAGE_OPENSSL),y)
	GIT_DEPENDENCIES += openssl
	GIT_CONF_OPT += --with-openssl
else
	GIT_CONF_OPT += --without-openssl
endif

ifeq ($(BR2_PACKAGE_PERL),y)
	GIT_DEPENDENCIES += perl
	GIT_CONF_OPT += --with-libpcre
else
	GIT_CONF_OPT += --without-libpcre
endif

ifeq ($(BR2_PACKAGE_CURL),y)
	GIT_DEPENDENCIES += curl
	GIT_CONF_OPT += --with-curl
else
	GIT_CONF_OPT += --without-curl
endif

ifeq ($(BR2_PACKAGE_EXPAT),y)
	GIT_DEPENDENCIES += expat
	GIT_CONF_OPT += --with-expat
else
	GIT_CONF_OPT += --without-expat
endif

ifeq ($(BR2_PACKAGE_LIBICONV),y)
	GIT_DEPENDENCIES += libiconv
	GIT_CONF_ENV += LIBS=-liconv
	GIT_CONF_OPT += --with-iconv=/usr/lib
else
	GIT_CONF_OPT += --without-iconv
endif

ifeq ($(BR2_PACKAGE_TCL),y)
	GIT_DEPENDENCIES += tcl
	GIT_CONF_OPT += --with-tcltk
else
	GIT_CONF_OPT += --without-tcltk
endif

# assume yes for these tests, configure will bail out otherwise
# saying error: cannot run test program while cross compiling
GIT_CONF_ENV += ac_cv_fread_reads_directories=yes \
	ac_cv_snprintf_returns_bogus=yes

$(eval $(autotools-package))
