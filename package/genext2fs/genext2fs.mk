#############################################################
#
# genext2fs
#
#############################################################

GENEXT2FS_VERSION=1.4.1
GENEXT2FS_SOURCE=genext2fs-$(GENEXT2FS_VERSION).tar.gz
GENEXT2FS_SITE:=http://downloads.sourceforge.net/project/genext2fs/genext2fs/$(GENEXT2FS_VERSION)

$(eval $(autotools-package))
$(eval $(host-autotools-package))
