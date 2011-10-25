AUTOMAKE_OPTIONS=foreign

#default including path
PIC_CORE_INC=-I../include -I./include
## ====================for pixel utilities===========================
PIXUTIL=pixutil/transforms.c

## ====================for decoder files=============================
DEC_DIR=dec

if SUPPORT_BMP
BMP_SRCS =	$(DEC_DIR)/bmp.c
else
BMP_SRCS =
endif

if SUPPORT_AMLJPEG
AMLJPEG_SRCS =	$(DEC_DIR)/amljpeg.c
AMLJPEG_LIBS = -lamljpeg
else
AMLJPEG_SRCS =
AMLJPEG_LIBS =
endif

if SUPPORT_JPEG
JPEG_SRCS =	$(DEC_DIR)/jpeg.c
JPEG_LIBS = -ljpeg
else
JPEG_SRCS =
JPEG_LIBS =
endif

if SUPPORT_PNG
PNG_SRCS =	$(DEC_DIR)/png.c
PNG_LIBS =  -lpng12 -lz
else
PNG_SRCS =
PNG_LIBS =
endif

if SUPPORT_GIF
GIF_SRCS =	$(DEC_DIR)/gif.c
GIF_LIBS =  -lungif
else
GIF_SRCS =
GIF_LIBS =
endif

DECODER_SRC=$(BMP_SRCS) $(JPEG_SRCS) $(PNG_SRCS) $(GIF_SRCS) $(AMLJPEG_SRCS)
DECODER_LIBS=$(JPEG_LIBS) $(PNG_LIBS) $(GIF_LIBS) $(AMLJPEG_LIBS)

VO_DIR=vo

## ====================for display==========================
if SUPPORT_FB
FB_SRC=$(VO_DIR)/vo_fb.c
else
FB_SRC=
endif

if SUPPORT_GLES
if A_SYS_WIN32
GLES_LOCAL_DEP= $(VO_DIR)/gles/win32_runtime.c 
else 
GLES_LOCAL_DEP= $(VO_DIR)/gles/linux_runtime.c
endif

GLES_SRC=$(VO_DIR)/vo_gles.c $(VO_DIR)/gles/egl_runtime.c $(VO_DIR)/gles/matrix.c \
	 $(VO_DIR)/gles/shaders.c $(VO_DIR)/gles/statics.c $(VO_DIR)/gles/textures.c \
	  $(VO_DIR)/gles/gles_common.c $(GLES_LOCAL_DEP)
	 
else
GLES_SRC=
endif

VO_SRC=$(VO_DIR)/vo.c  $(GLES_SRC)  $(FB_SRC) 

## ====================for core app=========================
SOURCE= $(PIXUTIL) $(DECODER_SRC) $(VO_SRC) pic_app.c

lib_LIBRARIES = libampic.a $(GLES_LIB)

libampic_a_SOURCES = $(SOURCE)
libampic_a_CPPFLAGS= $(PIC_CORE_INC)
##libampic_a_LDFLAGS = $(DECODER_LIBS)