##############################################################################
# testunit template Makefile
# Copyright (c) 2000 Martin Jolicoeur, Greg Haerr
##############################################################################

OBJS :=irremote.o config.o parsefile.o

TARGET = amremote_config

TOP=$(shell pwd)
export TOP



all:$(TARGET) 
	@echo amremote_config build completed
	
$(TARGET):$(OBJS) 
	$(CC)	$(CFLAGS) $(LDFLAGS) $(OBJS:%.o=%.c)  -o $(TARGET)

 
clean:
	rm -rf *.a *.o $(TARGET)
