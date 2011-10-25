#############################################################
#
# ampicplayer
#
#############################################################

AMPICPLAYER_VERSION:=1.0b
AMPICPLAYER_SOURCE=ampicplayer-$(AMPICPLAYER_VERSION).tar.gz
AMPICPLAYER_SITE=./package/amlogic/ampicplayer/ampicplayer
AMPICPLAYER_SITE_METHOD=cp
AMPICPLAYER_CONF_OPTS=--enable-jpeg --enable-amljpeg --enable-png --disable-gles
AMPICPLAYER_DEPENDS=libpng jpeg libungif

$(eval $(call AUTOTARGETS,package/amlogic,ampicplayer))
