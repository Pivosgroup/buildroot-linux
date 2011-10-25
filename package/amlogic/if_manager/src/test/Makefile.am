#########################################################################
#  
#   Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
# 
#   Description: the Makefile.am for AUTOMAKE, to generate ifm_test
# 
#   Author: tianyu.li
# 
##########################################################################

AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = ifm_test

LIBS += -lpthread

INCLUDES = -I../include

export INCLUDES

ifm_test_SOURCES = ./ifm_test.c

ifm_test_LDADD = ../src/.libs/libifm.so

