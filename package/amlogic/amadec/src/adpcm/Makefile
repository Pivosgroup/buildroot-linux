
LIBNAME_COMMON = libadpcm.a

SRC_OBJ=adpcm_decode.o
SRC_C=$(SRC_OBJ:%.o=%.c)

OBJ=libadpcm.a
HEADERS1=$(shell ls ../include/*.h)
HEADERS=$(shell ls *.h) $(HEADERS1)

$(OBJ): $(SRC_C) $(HEADERS)
	$(CC) -c $(CFLAGS)  $(SRC_C) -ltermcap -lm -o $@

clean:
	-rm -rf *.o $(OBJ)
