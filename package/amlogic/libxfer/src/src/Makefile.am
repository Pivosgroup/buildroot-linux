##########################################################################
#  
#   Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
# 
#   Description: the interface of Transfer Manager
# 
#   Author: Peifu Jiang
# 
##########################################################################


VERSIONINFO=-version-info 0:2:0
#   1. Start with version information of ‘0:0:0’ for each libtool library.
#   2. Update the version information only immediately before a public release of your software. 
#      More frequent updates are unnecessary, and only guarantee that the current interface number gets larger faster.
#   3. If the library source code has changed at all since the last update, then increment revision (‘c:r:a’ becomes ‘c:r+1:a’).
#   4. If any interfaces have been added, removed, or changed since the last update, increment current, and set revision to 0.
#   5. If any interfaces have been added since the last public release, then increment age.
#   6. If any interfaces have been removed since the last public release, then set age to 0. 

AUTOMAKE_OPTIONS = foreign
lib_LTLIBRARIES=libxfer.la
#ACLOCAL_AMFLAGS=-I m4
INCLUDES=-I../include
export INCLUDES

LIBS += -lpthread -lcurl

libxfer_la_SOURCES=./common/transfer_manager.c \
				  ./common/transfer_utils.c \
				  ./common/transfer_utils.h \
				  ./common/transfer_ctrl.c \
				  ./common/transfer_ctrl.h \
				  ./common/xfer_debug.c \
				  ./common/xfer_debug.h \
				  ./common/llist.h \
				  ./common/listop.h \
				  ./http/http_manager.c \
				  ./http/http_manager.h \
				  ./http/http_demo.c \
				  ./http/http_curl.c \
				  ./http/http_curl.h \
				  ./thunder/thunder_manager.c \
				  ./thunder/thunder_manager.h \
				  ./thunder/embed_thunder.h 

libxfer_la_LDFLAGS=$(VERSIONINFO)
if DEMO
libxfer_la_LDFLAGS+=-L../ixml/.libs -lixml
HAS_DEMO = -DHAS_OFFLINE_DEMO
endif

if THUNDER
libxfer_la_LDFLAGS+=-L./thunder -lembed_thunder
HAS_THUNDER = -DHAS_MODULE_THUNDER
endif

libxfer_la_CPPFLAGS=$(HAS_THUNDER) $(HAS_DEMO)

if DEBUG
DEBUGFLAGS = -DXFER_DEBUG
libxfer_la_CPPFLAGS+=-g $(DEBUGFLAGS)
endif

pkginclude_HEADERS=../include/transfer_manager.h \
				../include/xfer_common.h \
				../include/xfer_error.h \
				../include/transfer_def.h

if THUNDER
thunderlibdir=$(libdir)
thunderlib_DATA=./thunder/libembed_thunder.so
endif
EXTRA_DIST=./thunder/libembed_thunder.so 
