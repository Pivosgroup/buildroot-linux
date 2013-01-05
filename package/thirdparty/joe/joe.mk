#############################################################
#
# joe
#
#############################################################

JOE_VERSION = 3.7
JOE_SOURCE = joe-$(JOE_VERSION).tar.gz
JOE_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/joe-editor
JOE_INSTALL_STAGING = NO
JOE_INSTALL_TARGET = YES
JOE_DEPENDENCIES = ncurses

JOE_CONF_OPT += --program-transform-name=''

define JOE_INSTALL_ETC
  cp -rf package/joe/joerc $(TARGET_DIR)/etc/joe/.
endef

JOE_POST_INSTALL_TARGET_HOOKS += JOE_INSTALL_ETC

$(eval $(call AUTOTARGETS,package/thirdparty,joe))
