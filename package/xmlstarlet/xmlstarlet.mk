################################################################################
#
# xmlstarlet
#
################################################################################

XMLSTARLET_VERSION = 1.5.0
XMLSTARLET_SITE = http://downloads.sourceforge.net/project/xmlstar/xmlstarlet/$(XMLSTARLET_VERSION)
XMLSTARLET_LICENSE = MIT
XMLSTARLET_LICENSE_FILES = COPYING

XMLSTARLET_DEPENDENCIES += libxml2 libxslt \
	$(if $(BR2_PACKAGE_LIBICONV),libiconv)

XMLSTARLET_CONF_OPT += --disable-static-libs \
	--with-libxml-prefix=${STAGING_DIR}/usr \
	--with-libxslt-prefix=${STAGING_DIR}/usr \
	--with-libiconv-prefix=${STAGING_DIR}/usr

$(eval $(autotools-package))
