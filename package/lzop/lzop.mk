#############################################################
#
# lzop
#
#############################################################
LZOP_VERSION = 1.03
LZOP_SOURCE = lzop-$(LZOP_VERSION).tar.gz
LZOP_SITE = http://www.lzop.org/download/
LZOP_LICENSE = GPLv2+
LZOP_LICENSE_FILES = COPYING
LZOP_DEPENDENCIES = lzo

$(eval $(autotools-package))
