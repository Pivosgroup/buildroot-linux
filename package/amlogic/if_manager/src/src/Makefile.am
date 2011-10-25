##########################################################################
#  
#   Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
# 
#   Description: source Makefile.am for AUTOMAKE 
# 
#   Author: tianyu.li
# 
##########################################################################

AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES = libifm.la

LIBS += -lpthread

INCLUDES=-I../include

export INCLULDES

libifm_la_SOURCES=./if_manager.c \
                  ./network_setting.c \
                  ./network_setting.h 

pkginclude_HEADERS=../include/if_manager.h \
		   ../include/ifm_def.h




