################################################################################
#
# ngircd
#
################################################################################

NGIRCD_VERSION = 20.3
NGIRCD_SITE = ftp://ftp.berlios.de/pub/ngircd/
NGIRCD_DEPENDENCIES = zlib
NGIRCD_LICENSE = GPLv2+
NGIRCD_LICENSE_FILES = COPYING

$(eval $(autotools-package))
