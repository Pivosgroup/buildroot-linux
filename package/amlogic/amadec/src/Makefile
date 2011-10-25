M_PREFIX=$(PREFIX)

MP3LIB=mp3lib/libmp3.a
PCMLIB=pcm/libpcm.a
ADPCMLIB=adpcm/libadpcm.a
AACLIB=faad2/libfaad.a
A52LIB=liba52/liba52.a
AUDIO_OUT_LIB=audio_out/libout.a

AMADECD_OBJS=amadecd.o adecproc.o adec.o log.o feeder.o codec_mgt.o audiodsp_ctl.o sound_ctrl.o comm_socket_udp.o taskmngr.o sound_spectrum.o
AMADECC_OBJS=amadecc.o
AMADEC_DIST=amadecc amadecd

HEADERS=$(shell ls include/*.h)
DEPENDS=$(HEADERS)
INSTALL_DIR=${M_PREFIX}/bin

OBJS := $(AMADECD_OBJS) $(AMADECC_OBJS) 
LDFLAGS += -lpthread -lm -lasound -lrt

TARGET_DIR=$(shell pwd)/../../target
STAGING_DIR=$(shell pwd)/../../staging
CFLAGS+=-O2
CFLAGS+=-I$(shell pwd)/include -I${STAGING_DIR}/include -I${STAGING_DIR}/usr/include -L${TARGET_DIR}/lib -L${TARGET_DIR}/usr/lib





export CC CFLAGS

all: $(AMADEC_DIST) 
	make install
	
amadecc: $(AMADECC_OBJS:%.o=%.c)  $(DEPENDS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(AMADECC_OBJS:%.o=%.c) -o $@

amadecd: $(AMADECD_OBJS)  ${MP3LIB} $(PCMLIB) $(ADPCMLIB) $(AACLIB) $(A52LIB) ${AUDIO_OUT_LIB} $(DEPENDS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(AMADECD_OBJS:%.o=%.c) ${MP3LIB} $(PCMLIB) $(ADPCMLIB) $(AACLIB) $(A52LIB) ${AUDIO_OUT_LIB} -o $@  

	
	
${MP3LIB}:force
	@make -C mp3lib 

$(PCMLIB):force
	@make -C pcm

$(ADPCMLIB):force
	@make -C adpcm

$(AACLIB):force
	@make  -C faad2

$(A52LIB):force
	@make	-C liba52

${AUDIO_OUT_LIB}:force
	@make -C audio_out  

install:
	-install -m 555 ${AMADEC_DIST} $(INSTALL_DIR)


force:

clean:
	-rm -rf *.o ${AMADEC_DIST} ${MP3LIB} $(PCMLIB) $(ADPCMLIB) $(AACLIB) $(A52LIB)
	@make -C ampcm clean
	@make -C audio_out clean
	@make -C faad2 clean
	@make -C liba52 clean

	
