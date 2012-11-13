CC=${HOST_GCC}

export CC
all:
	-make -C amadec install
	-make -C amcodec install
	-make -C amffmpeg
	-make -C amffmpeg install
	-make -C amplayer install

install:all

clean:
	-make -C amadec clean
	-make -C amcodec clean
	-make -C amffmpeg clean
	-make -C amplayer clean
