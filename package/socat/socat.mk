#############################################################
#
# socat
#
#############################################################

SOCAT_VERSION = 2.0.0-b5
SOCAT_SOURCE = socat-$(SOCAT_VERSION).tar.bz2
SOCAT_SITE = http://www.dest-unreach.org/socat/download/
SOCAT_CONF_ENV = sc_cv_termios_ispeed=no \
		 sc_cv_sys_crdly_shift=9 \
		 sc_cv_sys_tabdly_shift=11 \
		 sc_cv_sys_csize_shift=4

# We need to run autoconf to regenerate the configure script, in order
# to ensure that the test checking linux/ext2_fs.h works
# properly. However, the package only uses autoconf and not automake,
# so we can't use the normal autoreconf logic.

SOCAT_DEPENDENCIES = host-autoconf

define SOCAT_RUN_AUTOCONF
	(cd $(@D); $(HOST_DIR)/usr/bin/autoconf)
endef

SOCAT_PRE_CONFIGURE_HOOKS += SOCAT_RUN_AUTOCONF

$(eval $(call AUTOTARGETS))
