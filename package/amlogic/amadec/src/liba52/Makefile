LIBNAME_COMMON=liba52.a

OBJS= a52_decode.o \
      resample.o \
      bit_allocate.o \
      bitstream.o \
      downmix.o \
      imdct.o \
      parse.o \

CFLAGS+=-I$(shell pwd)/include

all:$(LIBNAME_COMMON)

$(LIBNAME_COMMON):$(OBJS)
	$(HOST_NAME)-ar rcs $@ $(OBJS)

$(OBJS):%.o:%.c
	$(CC) -c $(DEFS) $(CFLAGS) $< -o $@


clean:
	-rm -rf $(OBJS)
