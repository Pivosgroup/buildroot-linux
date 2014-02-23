#############################################################
#
# make
#
#############################################################
MAKE_VERSION:=3.81
MAKE_SOURCE:=make-$(MAKE_VERSION).tar.bz2
MAKE_SITE:=$(BR2_GNU_MIRROR)/make

MAKE_DEPENDENCIES = $(if $(BR2_NEEDS_GETTEXT_IF_LOCALE),gettext libintl)

MAKE_CONF_ENV = make_cv_sys_gnu_glob=no \
		GLOBINC='-I$(@D)/glob' \
		GLOBLIB=glob/libglob.a

$(eval $(call AUTOTARGETS))
