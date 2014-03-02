################################################################################
#
# libcap-ng
#
################################################################################

LIBCAP_NG_VERSION = 0.7.3
LIBCAP_NG_SITE = http://people.redhat.com/sgrubb/libcap-ng/
LIBCAP_NG_LICENSE = GPLv2+ LGPLv2.1+
LIBCAP_NG_LICENSE_FILES = COPYING COPYING.LIB
LIBCAP_NG_INSTALL_STAGING = YES

LIBCAP_NG_CONF_ENV = ac_cv_prog_swig_found=no
LIBCAP_NG_CONF_OPT = --without-python

$(eval $(autotools-package))
