

SRC_ALSA_OBJ=alsa_out.o
SRC_ALSA_C=$(SRC_ALSA_OBJ:%.o=%.c)

SRC_OSS_OBJ=oss_out.o
SRC_OSS_C=$(SRC_OSS_OBJ:%.o=%.c)

OBJ=libout.a
HEADERS1=$(shell ls ../include/*.h)
HEADERS=$(HEADERS1)

all:$(OBJ)


$(OBJ): $(SRC_ALSA_OBJ) $(SRC_OSS_OBJ)
	$(HOST_NAME)-ar -rcs $@ $(SRC_ALSA_OBJ) $(SRC_OSS_OBJ)

$(SRC_ALSA_OBJ): $(SRC_ALSA_C)  $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $(SRC_ALSA_C) -ltermcap -lm

$(SRC_OSS_OBJ):  $(SRC_OSS_C)   $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $(SRC_OSS_C) -ltermcap -lm


clean:
	-rm -rf *.o $(OBJ)
	
