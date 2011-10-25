# object building.
ifeq ($(ARM), 1)
CC=arm-none-linux-gnueabi-gcc
AR=arm-none-linux-gnueabi-ar
else
CC=gcc
AR=ar
endif

TARGET=output
OBJECTS=output.o 
ifeq ($(ARM), 1)
CFLAGS+=-DARM 
LDFLAGS += -lm -lamljpeg
else
LDFLAGS += -lm 
CFLAGS+=-std=c99
endif
CFLAGS+=-O2
LOCAL_PATH=./../src
CFLAGS+= -g -I$(LOCAL_PATH)  -L$(LOCAL_PATH) -L./ -I${STAGING_DIR}/usr/include
$(TARGET):$(OBJECTS) 
	$(CC) $(CFLAGS)  $(OBJECTS) -o $(TARGET) $(LDFLAGS)
OBJECTS:%.o:%.c
	$(CC) -c $(CFLAGS)  $< -o $@ 
clean:
	-rm -f *.o
	-rm -f $(TARGET)

	
