##############################################################################
# testunit template Makefile
# Copyright (c) 2000 Martin Jolicoeur, Greg Haerr
##############################################################################

OBJS :=irremote.o config.o parsefile.o

TARGET = amremote_config

TOP=$(shell pwd)
export TOP



all:$(TARGET) keytest
	@echo amremote_config build completed
	
keytest:irkey.o
	$(CC)	$(CFLAGS) $(LDFLAGS) $(OBJS:%.o=%.c)  -o keytest

$(TARGET):$(OBJS) 
	$(CC)	$(CFLAGS) $(LDFLAGS) $(OBJS:%.o=%.c)  -o $(TARGET)

 
clean:
	rm -rf *.a *.o $(TARGET)
