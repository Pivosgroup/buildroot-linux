XBMCPVR_VERSION = 0482a89a81f3f3de8f61ac620abcd79c116e8a9d
XBMCPVR_SITE = git://github.com/opdenkamp/xbmc-pvr-addons.git
XBMCPVR_AUTORECONF = YES
XBMCPVR_INSTALL_STAGING = YES
XBMCPVR_INSTALL_TARGET = YES

XBMCPVR_CONF_ENV += MYSQL_CONFIG=$(TARGET_DIR)/usr/bin/mysql_config
XBMCPVR_CONF_OPT += --enable-addons-with-dependencies

$(eval $(call AUTOTARGETS,package/thirdparty,xbmcpvr))

