P_FILE=if_manager-1.1.0
FILE_DIR=if_manager-1.1.0

#P_URL=http://10.68.11.58/packages_download/if_manager-1.0.0.tar.gz



#P_GET_CMD=${HTTP_GET}
#P_GET_FLAGS=${HTTP_GET_FLAGS}


#TAR_CMD=${CMD_GZ}
#TAR_FLAGS=${CMD_GZ_FLAGS}

#TRY_MAX=2

DEPENDS=

CONFIG_FLAGS=--prefix=${PREFIX} --host=${HOST_NAME} --enable-debug=no
CONFIG_FLAGS+=CFLAGS='-mA7 -I${PREFIX}/include -L${PREFIX}/lib' CPPFLAGS='-I${PREFIX}/include'

P_FILE_F=${PKG_DIR}/${P_FILE}
BUILD_DIR=${PKG_BUILD_DIR}/${FILE_DIR}

TAR_DIR=${PKG_BUILD_DIR}/${FILE_DIR}

.PHONY:all configure config before_cmd install
all:before_cmd configure 
	make -C ${BUILD_DIR}
	make install
	

configure:config
	cd ${BUILD_DIR}/&& \
	export CC=${HOST_GCC} && \
	./configure  ${CONFIG_FLAGS}
	

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

CONFIG_GEN_FILE=${BUILD_DIR}/config.mk
CONFIG_DEP_FILE=${BUILD_DIR}/configure
UNZIP_GEN_FILE=${BUILD_DIR}/configure
UNZIP_DEP_FILE=${P_FILE_F}

	
#include $(MAKEFILES_DIR)/package.rules
