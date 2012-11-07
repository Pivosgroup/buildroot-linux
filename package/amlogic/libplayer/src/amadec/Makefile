M_PREFIX=$(PREFIX)

TARGET_OBJS  = adec-external-ctrl.o
TARGET_OBJS += adec-internal-mgt.o
TARGET_OBJS += adec-message.o
TARGET_OBJS += adec-pts-mgt.o feeder.o
TARGET_OBJS += dsp/audiodsp-ctl.o
TARGET_OBJS += audio_out/aml_resample.c
TARGET_OBJS += audio_out/alsa-out.o

TARGET=libamadec.so

INSTALL_DIR=${M_PREFIX}/lib

LDFLAGS += -shared -lpthread -lm -lasound -lrt

TARGET_DIR=$(shell pwd)/../../../target
STAGING_DIR=$(shell pwd)/../../../staging
CFLAGS+=-O2
CFLAGS+=-I$(shell pwd) -I$(shell pwd)/include -I${STAGING_DIR}/include -I${STAGING_DIR}/usr/include -L${TARGET_DIR}/lib -L${TARGET_DIR}/usr/lib
CFLAGS+=-DALSA_OUT -DENABLE_WAIT_FORMAT

export CC CFLAGS

all: $(TARGET) 

$(TARGET): $(TARGET_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(TARGET_OBJS:%.o=%.c) -o $@  

install:$(TARGET)
	-install -m 555 ${TARGET} $(INSTALL_DIR)

force:

clean:
	-rm -rf *.o ${AMADEC_DIST} ${MP3LIB} $(PCMLIB) $(ADPCMLIB) $(AACLIB) $(A52LIB)
