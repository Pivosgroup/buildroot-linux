#############################################################
#
# netcat
#
#############################################################

NETCAT_VERSION:=0.7.1
NETCAT_SOURCE:=netcat-$(NETCAT_VERSION).tar.gz
NETCAT_SITE=http://$(BR2_SOURCEFORGE_MIRROR).dl.sourceforge.net/sourceforge/netcat
NETCAT_AUTORECONF:=NO
NETCAT_INSTALL_STAGING:=NO
NETCAT_INSTALL_TARGET:=YES

$(eval $(call AUTOTARGETS))
