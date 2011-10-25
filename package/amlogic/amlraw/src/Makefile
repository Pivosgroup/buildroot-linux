CC=${HOST_GCC}
TARGET=libamlraw.a
AMLRAW_LIB=amlraw.o cmem.o crop.o  scaler.o convert.o
HEADERS=$(shell ls include/*.h)
#LDFLAGS += -lpthread -lm -lrt
LDFLAGS += -lm -lrt
CFLAGS+=-O2
CFLAGS+=-DNO_JPEG -DNO_LCMS
LOCAL_PATH=./libaml
CFLAGS+=-I$(shell pwd)/include -I${STAGING_DIR}/include -I${STAGING_DIR}/usr/include -I$(LOCAL_PATH) -L${TARGET_DIR}/lib -L${TARGET_DIR}/usr/lib

all:$(TARGET)
	-cp amlraw.h $(STAGING)/include/
	-cp $(TARGET) $(STAGING)/lib
	

$(TARGET):$(AMLRAW_LIB)
	$(HOST_NAME)-ar rcs $@ $(AMLRAW_LIB)
$(AMLRAW_LIB):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	-rm -f *.o
	-rm -f $(TARGET)
