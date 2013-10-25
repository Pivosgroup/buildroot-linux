XBMCPVR_VERSION = 0c0ed7de451a1e61938eb125e01a8d1a972ec79c
XBMCPVR_SITE = git://github.com/Pivosgroup/pivos-pvr-addons.git
XBMCPVR_AUTORECONF = YES
XBMCPVR_INSTALL_STAGING = YES
XBMCPVR_INSTALL_TARGET = YES

XBMCPVR_CONF_ENV += MYSQL_CONFIG=$(TARGET_DIR)/usr/bin/mysql_config
XBMCPVR_CONF_OPT += --enable-addons-with-dependencies

$(eval $(call AUTOTARGETS,package/thirdparty,xbmcpvr))

