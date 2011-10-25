CC=${HOST_GCC}

export CC
all:
	make -f Makefile.mk  TARGET=libamplayer.so
	make -f Makefile.mk  TARGET=libamcontroler.so
	-make -C utils  
	cp utils/amplayer2 .
	

install:
	make -f Makefile.mk TARGET=libamplayer.so  install
	make -f Makefile.mk TARGET=libamcontroler.so install
	-make -C utils  install
	
clean:
	-make -f Makefile.mk clean
	-make -C utils clean clean



	
