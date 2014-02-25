#############################################################
#
# sylpheed
#
#############################################################
SYLPHEED_VERSION_MAJOR = 3.2
SYLPHEED_VERSION_MINOR = 0
SYLPHEED_VERSION = $(SYLPHEED_VERSION_MAJOR).$(SYLPHEED_VERSION_MINOR)
SYLPHEED_SOURCE = sylpheed-$(SYLPHEED_VERSION).tar.bz2
SYLPHEED_SITE = http://sylpheed.sraoss.jp/sylpheed/v$(SYLPHEED_VERSION_MAJOR)
SYLPHEED_LICENSE = GPLv2+ (executables), LGPLv2.1+ (library, attachment plugin)
SYLPHEED_LICENSE_FILES = COPIYNG COPYING.LIB

SYLPHEED_CONF_OPT = --disable-gtkspell

SYLPHEED_DEPENDENCIES = host-pkgconf libgtk2

# Remove the -I$(includedir) from the Makefiles
# because it refers to the host /usr/include.
define SYLPHEED_PRECONFIGURE
	for i in $$(find $(@D) -name "Makefile*"); do \
		sed -i 's:-I$$(includedir)::g' $$i; \
	done
endef

SYLPHEED_PRE_CONFIGURE_HOOKS += SYLPHEED_PRECONFIGURE

ifeq ($(BR2_PACKAGE_OPENSSL),y)
SYLPHEED_DEPENDENCIES += openssl
SYLPHEED_CONF_OPT += --enable-ssl
else
SYLPHEED_CONF_OPT += --disable-ssl
endif

$(eval $(autotools-package))
