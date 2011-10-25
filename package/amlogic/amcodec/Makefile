
FILE_DIR=amcodec


DEPENDS=

CONFIG_FLAGS=

TAR_DIR=${PKG_BUILD_DIR}/${FILE_DIR}

P_FILE_F=${PKG_DIR}/${P_FILE}
BUILD_DIR=${PKG_BUILD_DIR}/${FILE_DIR}

.PHONY:configure	all config  before_cmd install clean distclean


all:before_cmd config  
	make -C ${BUILD_DIR} CC=${HOST_GCC}
	make install
	
configure:config


config:get_file

get_file:
	@if [ ! -d "${TAR_DIR}" ]; then \
		mkdir ${TAR_DIR}; \
	fi;
	cp -rf ${PKG_TOP_DIR}/${FILE_DIR}/src/* ${PKG_BUILD_DIR}/${FILE_DIR}
	

install:
	make -C ${BUILD_DIR} CC=${HOST_GCC} install

distclean:clean
	
clean:
	-make -C ${BUILD_DIR} CC=${HOST_GCC} clean


before_cmd:depends

depends:
	@if [   "${DEPENDS}" != "" ]; then \
                cd ${PKG_TOP_DIR};make lib_depends TOP_DEPENDS="${DEPENDS}";    \
        fi
#include $(MAKEFILES_DIR)/package.rules

