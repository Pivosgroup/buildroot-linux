#############################################################
#
# joe
#
#############################################################

JOE_VERSION = 3.7
JOE_SOURCE = joe-$(JOE_VERSION).tar.gz
JOE_SITE = http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/joe-editor
JOE_INSTALL_TARGET = YES
JOE_DEPENDENCIES = ncurses
$(eval $(call AUTOTARGETS,package/thirdparty,joe))
