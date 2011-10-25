
LIBNAME_COMMON = libmp3.a

SRC_OBJ=sr_am.o
SRC_C=$(SRC_OBJ:%.o=%.c)

OBJ=libmp3.a
HEADERS1=$(shell ls ../include/*.h)
HEADERS=$(shell ls *.h) $(HEADERS1)

$(OBJ): $(SRC_C) $(HEADERS)
	$(CC) -c $(CFLAGS)  $(SRC_C) -ltermcap -lm -o $@

clean:
	-rm -rf *.o $(OBJ)
