CC=${HOST_GCC}

TARGET=libcmem.a

CMEM_LIB=cmem.o 
HEADERS=$(shell ls include/*.h)
#LDFLAGS += -lpthread -lm -lrt
LDFLAGS += -lm -lrt
CFLAGS+=-O2
CFLAGS+=-I$(shell pwd)/include -I${STAGING_DIR}/include -I${STAGING_DIR}/usr/include -L${TARGET_DIR}/lib -L${TARGET_DIR}/usr/lib

all:$(TARGET)
	-cp cmem.h $(STAGING)/include
	-cp $(TARGET) $(STAGING)/lib
	

$(TARGET):$(CMEM_LIB)
	$(HOST_NAME)-ar rcs $@ $(CMEM_LIB)
$(CMEM_LIB):%.o:%.c
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f $(STAGING)/include/cmem.h
	-rm -f $(STAGING)/lib/$(TARGET)



	
