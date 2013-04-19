XBMCPVR_VERSION = d37ec1d7c536654fd4ffc3e40d116586d2a57283
XBMCPVR_SITE = git://github.com/opdenkamp/xbmc-pvr-addons.git
XBMCPVR_AUTORECONF = YES
XBMCPVR_INSTALL_STAGING = YES
XBMCPVR_INSTALL_TARGET = YES

XBMCPVR_CONF_ENV += MYSQL_CONFIG=$(TARGET_DIR)/usr/bin/mysql_config
XBMCPVR_CONF_OPT += --enable-addons-with-dependencies

$(eval $(call AUTOTARGETS,package/thirdparty,xbmcpvr))

