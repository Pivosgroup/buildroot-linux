#############################################################
#
# libenca
#
#############################################################
LIBENCA_VERSION = 1.9
LIBENCA_SITE = http://dl.cihar.com/enca
LIBENCA_SOURCE = enca-$(LIBENCA_VERSION).tar.bz2
LIBENCA_INSTALL_STAGING = YES
LIBENCA_INSTALL_TARGET = YES

LIBENCA_CONF_ENV += ac_cv_file__dev_random=yes
LIBENCA_CONF_ENV += ac_cv_file__dev_urandom=yes
LIBENCA_CONF_ENV += ac_cv_file__dev_arandom=yes
LIBENCA_CONF_ENV += ac_cv_file__dev_srandom=yes
LIBENCA_CONF_ENV += yeti_cv_func_scanf_modif_size_t=yes

define LIBENCA_MAKE_FIX
cd $(LIBENCA_DIR)/tools && $(HOSTCC) -o make_hash make_hash.c
endef

LIBENCA_POST_CONFIGURE_HOOKS += LIBENCA_MAKE_FIX
$(eval $(call AUTOTARGETS,package/thirdparty,libenca))
