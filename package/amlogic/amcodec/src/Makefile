
ROOTFS?=$(PREFIX)
LIB_DIR?=$(PREFIX)/lib
HEADERS_DIR?=$(INC_DIR)/include
CROSS=$(CC:%gcc=%)
QUITE_CMD=1

INSTALL_FLAGS=-m 755

CPP=$(CROSS)g++
AS=$(CROSS)as
AR=$(CROSS)ar
LD=$(CROSS)ld
NM=$(CROSS)nm
STRIP=$(CROSS)strip
OBJCOPY=$(CROSS)objcopy
export CC CPP AS AR LD NM STRIP OBJCOPY

TOPDIR=$(shell pwd)
SRC?=$(TOPDIR)
SRCTREE=$(SRC)
BUILDTREE=$(SRC)/out


TARGET?=libamcodec.so

DIRS=audio_ctl/ codec/ subtitle/

LIB_VERSION=
LIB_SUB_VERSION=.0.0

LDFLAGS=  -O2  -L$(PREFIX)/lib  -lm -lc  -shared -Wl,--shared -Wl,-soname,$(TARGET)$(LIB_VERSION)

INCLUDE=${SRCTREE}/include/
#CFLAGS=-O0  -gdwarf-2  -g
CFLAGS=-O2   -I$(SRC)/include  -fPIC




target_all=  $(TARGET)$(LIB_VERSION)$(LIB_SUB_VERSION)
all:$(target_all)

dirs=$(DIRS)

DIROBJ=$(dirs:%/=%-dir)


$(target_all):$(DIROBJ) 
	$(call ld_mk,$(OUT_DIR)$@,$^)




include $(SRCTREE)/rules.mk
export CC CPP AS AR LD TOPDIR SRCTREE  CFLAGS LDFLAGS Q

CLRDIR=$(dirs:%/=%-clrdir)
clean:$(CLRDIR)
	-rm -rf $(target_all)

install:$(target_all)
	install  $(INSTALL_FLAGS)	$(TARGET)$(LIB_VERSION)$(LIB_SUB_VERSION)  $(LIB_DIR)
	cp	-rf $(SRC)/include/*  $(HEADERS_DIR)/
	-rm  $(LIB_DIR)/$(TARGET)
	-ln -s $(TARGET)$(LIB_VERSION)$(LIB_SUB_VERSION)  $(LIB_DIR)/$(TARGET)$(LIB_VERSION)

	
