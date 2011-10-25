CC=${HOST_GCC}

export CC
all:
	make -f Makefile.mk  TARGET=libamplayer.so
	make -f Makefile.mk  TARGET=libamcontroler.so
	

install:
	make -f Makefile.mk TARGET=libamplayer.so  install
	make -f Makefile.mk TARGET=libamcontroler.so install
	
clean:
	-make -f Makefile.mk clean



	
