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
lib_LTLIBRARIES=libixml.la
#ACLOCAL_AMFLAGS=-I m4
INCLUDES=-I../include
export INCLUDES


libixml_la_SOURCES=./ixml.c \
				  ./node.c \
				  ./ixmlparser.c \
				  ./ixmlparser.h \
				  ./ixmlmembuf.c \
				  ./ixmlmembuf.h \
				  ./nodeList.c \
				  ./element.c \
				  ./attr.c \
				  ./document.c \
				  ./namedNodeMap.c \
				  ./xml_util.c \
				  ./xml_util.h

if DEBUG
DEBUGFLAGS = -DXFER_DEBUG
libixml_la_CPPFLAGS=-g $(DEBUGFLAGS)
endif

pkginclude_HEADERS=../include/ixml.h 

