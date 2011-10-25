AUTOMAKE_OPTIONS=foreign

#default including path
PIC_CORE_INC=-I../include 

PLAYER_LIB = $(top_srcdir)/pic_core/libampic.a

## ====================for decoder libs=============================
if SUPPORT_JPEG
JPEG_LIBS = -ljpeg
else
JPEG_LIBS =
endif

if SUPPORT_AMLJPEG
AMLJPEG_LIBS = -lamljpeg
else
AMLJPEG_LIBS =
endif

if SUPPORT_PNG
PNG_LIBS =  -lpng12 -lz
else
PNG_LIBS =
endif

if SUPPORT_GIF
GIF_LIBS =  -lungif
else
GIF_LIBS =
endif

if SUPPORT_GLES
GLES_LIB= -lGLESv2 -lEGL
else
GLES_SRC=
endif

DECODER_LIBS=$(JPEG_LIBS) $(PNG_LIBS) $(GIF_LIBS) $(AMLJPEG_LIBS)

VO_LIBS= $(GLES_LIB)

## ====================for core app=========================
bin_PROGRAMS = 
if WANT_PLAYER
 bin_PROGRAMS += ampicplayer
endif 

ampicplayer_SOURCES = main.c player_main.c player_util.c filelistop.c 
ampicplayer_CPPFLAGS= $(PIC_CORE_INC)
ampicplayer_LDADD = $(PLAYER_LIB) $(DECODER_LIBS) $(VO_LIBS)
