##########################################################################
#  
#   Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
# 
#   Description: the interface of Transfer Manager
# 
#   Author: Peifu Jiang
# 
##########################################################################

AUTOMAKE_OPTIONS = foreign
noinst_PROGRAMS=xfer_test
#ACLOCAL_AMFLAGS=-I m4
INCLUDES=-I../include
export INCLUDES

LIBS += -lcurl -lpthread

xfer_test_SOURCES=./xfer_test.c \
				  ./xfer_update.c \
				  ./xfer_update.h
xfer_test_LDADD=../src/.libs/libxfer.so

xfer_test_LDFLAGS=
if DEMO
xfer_test_LDFLAGS+=-L../ixml/.libs -lixml
HAS_DEMO = -DHAS_OFFLINE_DEMO
endif
if THUNDER
xfer_test_LDFLAGS+=-L../src/thunder -lembed_thunder
HAS_THUNDER = -DHAS_MODULE_THUNDER
endif
xfer_test_CPPFLAGS=$(HAS_THUNDER) $(HAS_DEMO)

if DEBUG
xfer_test_CPPFLAGS+=-g
endif

