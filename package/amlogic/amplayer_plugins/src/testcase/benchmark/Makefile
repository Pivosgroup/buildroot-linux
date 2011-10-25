#############################################################################
# Makefile for building: socket_client
#############################################################################

####### Compiler, tools and options

CC            = 

CFLAGS        = 
INCPATH       = -I../../include
LINK          = 
LFLAGS        = 
LIBS          = $(SUBLIBS) -L../../lib -lmpapi -lm -lrt -ldl -lpthread
AR            = 
RANLIB        = 

TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -sf
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory


####### Files

SOURCES       = mp_tools.c plist_mngr.c

DESTDIR       = 
TARGET        = ../bin/mp_tools

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

OBJECTS = mp_tools.o plist_mngr.o
.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(CC) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)



####### Sub-libraries

clean: 
	-$(DEL_FILE) $(TARGET) $(OBJECTS)

####### Compile

mp_tools.o: mp_tools.c 
	$(CC) -c $(CFLAGS) $(INCPATH) -o mp_tools.o mp_tools.c

plist_mngr.o: plist_mngr.c 
	$(CC) -c $(CFLAGS) $(INCPATH) -o plist_mngr.o plist_mngr.c	
####### Install

TESTDIR =/nfsroot/peter/workshop/output/rootfs/usr/local/app/bin
install:   
	-$(COPY) $(TARGET) $(TESTDIR)/. 

uninstall:  
	-$(DEL_FILE) $(TESTDIR)/$(TARGET)

