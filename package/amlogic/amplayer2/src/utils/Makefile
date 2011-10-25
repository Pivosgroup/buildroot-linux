target=amplayer2 simple_player cplay muti_threadplayer

amplayer2_s=amplayer2.c
simple_player_s=simple_player.c
cplay_s=cplay.c
muti_threadplayer_s=muti_threadplayer.c

targets=$(target)

LDFLAGS= -L../ -L$(PREFIX)/lib -lz -lavutil -lavformat -lavcodec -lm  -lpthread -lamcodec   -ldl -lamplayer -lamcontroler
CFLAGS=   -I../player/include -I ../control/include -I$(PREFIX)/include 
CFLAGS+=-Wall -Werror 
CFLAGS+=-O2

EXTERN_LIB=
all:$(targets)


install:all
	install -m 755 $(targets) $(PREFIX)/bin

$(targets):_force
	$(CC) $(CFLAGS) $(LDFLAGS) $($(@:%=%_s)) $(EXTERN_LIB)	-o $@

_force:
	
