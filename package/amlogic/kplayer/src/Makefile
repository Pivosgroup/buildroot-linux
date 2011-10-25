M_PREFIX=$(PREFIX)


KPLAYER_OBJS=kplayer.o sys_conf.o
#KPLAYER_OBJS=simple_player.o
#KPLAYER_DIST=amplayer
KPLAYER_DIST=kplayer

HEADERS=$(shell ls include/*.h)
DEPENDS=$(HEADERS)
INSTALL_DIR=${M_PREFIX}/bin

OBJS := $(KPLAYER_OBJS) $(KPLAYERC_OBJS) 
LDFLAGS += -lpthread -lm -lasound -lrt -lamplayer -lamcodec -lavformat -lavutil -lavcodec -lamadec

TARGET_DIR=$(shell pwd)/../../target
STAGING_DIR=$(shell pwd)/../../staging
CFLAGS+=-O2
CFLAGS+=-I$(shell pwd)/../libplayer/amffmpeg/ -I$(shell pwd)/../libplayer/amplayer/player/include -I${STAGING_DIR}/include -I${STAGING_DIR}/usr/include -L${TARGET_DIR}/lib -L${TARGET_DIR}/usr/lib





export CC CFLAGS

all: $(KPLAYER_DIST) 
	make install
	

kplayer: $(KPLAYER_OBJS)$(DEPENDS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(KPLAYER_OBJS:%.o=%.c) -o $@  

	
	




install:
	-install -m 555 ${KPLAYER_DIST} $(INSTALL_DIR)


force:

clean:
	-rm -rf *.o ${KPLAYER_DIST}
