CC=${HOST_GCC}

TARGET=libamljpeg.a

AMLJPEG_LIB=amljpeg.o cmem.o 
HEADERS=$(shell ls include/*.h)
#LDFLAGS += -lpthread -lm -lrt
LDFLAGS += -lm -lrt
CFLAGS+=-O2
LOCAL_PATH=./libaml
CFLAGS+=-I$(shell pwd)/include -I${STAGING_DIR}/include -I${STAGING_DIR}/usr/include -I$(LOCAL_PATH) -L${TARGET_DIR}/lib -L${TARGET_DIR}/usr/lib

all:$(TARGET)
	-cp amljpeg.h $(STAGING)/include/
	-cp aml_common.h $(STAGING)/include/
	-cp $(TARGET) $(STAGING)/lib
	

$(TARGET):$(AMLJPEG_LIB)
	$(HOST_NAME)-ar rcs $@ $(AMLJPEG_LIB)
$(AMLJPEG_LIB):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	-rm -f *.o
	-rm -f $(TARGET)



	
