CIFS_UTILS_VERSION = 5.4
CIFS_UTILS_SOURCE  = cifs-utils-$(CIFS_UTILS_VERSION).tar.bz2
CIFS_UTILS_SITE    = ftp://ftp.samba.org/pub/linux-cifs/cifs-utils/

define CIFS_UTILS_NO_WERROR
	$(SED) 's/-Werror//' $(@D)/Makefile.in
endef

CIFS_UTILS_POST_PATCH_HOOKS += CIFS_UTILS_NO_WERROR

$(eval $(call AUTOTARGETS))
