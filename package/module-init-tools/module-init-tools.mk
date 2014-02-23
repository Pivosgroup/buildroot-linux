#############################################################
#
# module-init-tools
#
#############################################################
MODULE_INIT_TOOLS_VERSION=3.12
MODULE_INIT_TOOLS_SOURCE=module-init-tools-$(MODULE_INIT_TOOLS_VERSION).tar.bz2
MODULE_INIT_TOOLS_SITE=$(BR2_KERNEL_MIRROR)/linux/utils/kernel/module-init-tools/
MODULE_INIT_TOOLS_CONF_ENV = ac_cv_prog_DOCBOOKTOMAN=''
MODULE_INIT_TOOLS_CONF_OPT = \
	--disable-static-utils \
	--disable-builddir \
	--program-transform-name=''

# module-init-tools-3.11-add-manpages-config-option.patch is modifying
# configure.ac and Makefile.am
MODULE_INIT_TOOLS_AUTORECONF=YES
HOST_MODULE_INIT_TOOLS_AUTORECONF=YES
HOST_MODULE_INIT_TOOLS_CONF_ENV = ac_cv_prog_DOCBOOKTOMAN=''

$(eval $(call AUTOTARGETS))
$(eval $(call AUTOTARGETS,host))

