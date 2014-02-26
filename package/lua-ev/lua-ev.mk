################################################################################
#
# lua-ev
#
################################################################################

LUA_EV_VERSION = 458165bdfe0c6eadc788813925f11a0e6a823845
LUA_EV_SITE = http://github.com/brimworks/lua-ev/tarball/$(LUA_EV_VERSION)
LUA_EV_DEPENDENCIES = lua libev
LUA_EV_LICENSE = MIT
LUA_EV_LICENSE_FILES = README
LUA_EV_CONF_OPT = -DINSTALL_CMOD="/usr/lib/lua"

$(eval $(cmake-package))
