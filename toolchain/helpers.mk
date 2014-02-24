# This Makefile fragment declares helper functions, usefull to handle
# non- buildroot-built toolchains, eg. purely external toolchains or
# toolchains (internally) built using crosstool-NG.

#
# Copy a toolchain library and its symbolic links from the sysroot
# directory to the target directory. Also optionaly strips the
# library.
#
# Most toolchains (CodeSourcery ones) have their libraries either in
# /lib or /usr/lib relative to their ARCH_SYSROOT_DIR, so we search
# libraries in:
#
#  $${ARCH_LIB_DIR}
#  usr/$${ARCH_LIB_DIR}
#
# Buildroot toolchains, however, have basic libraries in /lib, and
# libstdc++/libgcc_s in /usr/<target-name>/lib(64), so we also need to
# search libraries in:
#
#  usr/$(TOOLCHAIN_EXTERNAL_PREFIX)/$${ARCH_LIB_DIR}
#
# Linaro toolchains have most libraries in lib/<target-name>/, so we
# need to search libraries in:
#
#  $${ARCH_LIB_DIR}/$(TOOLCHAIN_EXTERNAL_PREFIX)
#
# And recent Linaro toolchains have the GCC support libraries
# (libstdc++, libgcc_s, etc.) into a separate directory, outside of
# the sysroot, that we called the "SUPPORT_LIB_DIR", into which we
# need to search as well.
#
# Thanks to ARCH_LIB_DIR we also take into account toolchains that
# have the libraries in lib64 and usr/lib64.
#
# Please be very careful to check the major toolchain sources:
# Buildroot, Crosstool-NG, CodeSourcery and Linaro before doing any
# modification on the below logic.
#
# $1: arch specific sysroot directory
# $2: support libraries directory (can be empty)
# $3: library directory ('lib' or 'lib64') from which libraries must be copied
# $4: library name
# $5: destination directory of the libary, relative to $(TARGET_DIR)
#
copy_toolchain_lib_root = \
	ARCH_SYSROOT_DIR="$(strip $1)"; \
	SUPPORT_LIB_DIR="$(strip $2)" ; \
	ARCH_LIB_DIR="$(strip $3)" ; \
	LIB="$(strip $4)"; \
	DESTDIR="$(strip $5)" ; \
 \
	for dir in \
		$${ARCH_SYSROOT_DIR}/$${ARCH_LIB_DIR}/$(TOOLCHAIN_EXTERNAL_PREFIX) \
		$${ARCH_SYSROOT_DIR}/usr/$(TOOLCHAIN_EXTERNAL_PREFIX)/$${ARCH_LIB_DIR} \
		$${ARCH_SYSROOT_DIR}/$${ARCH_LIB_DIR} \
		$${ARCH_SYSROOT_DIR}/usr/$${ARCH_LIB_DIR} \
		$${SUPPORT_LIB_DIR} ; do \
		LIBSPATH=`find $${dir} -maxdepth 1 -name "$${LIB}.*" 2>/dev/null` ; \
		if test -n "$${LIBSPATH}" ; then \
			break ; \
		fi \
	done ; \
	for LIBPATH in $${LIBSPATH} ; do \
		LIBNAME=`basename $${LIBPATH}`; \
		LIBDIR=`dirname $${LIBPATH}` ; \
		while test \! -z "$${LIBNAME}" ; do \
			LIBPATH=$${LIBDIR}/$${LIBNAME} ; \
			rm -fr $(TARGET_DIR)/$${DESTDIR}/$${LIBNAME}; \
			mkdir -p $(TARGET_DIR)/$${DESTDIR}; \
			if test -h $${LIBPATH} ; then \
				cp -d $${LIBPATH} $(TARGET_DIR)/$${DESTDIR}/; \
			elif test -f $${LIBPATH}; then \
				$(INSTALL) -D -m0755 $${LIBPATH} $(TARGET_DIR)/$${DESTDIR}/$${LIBNAME}; \
			else \
				exit -1; \
			fi; \
			LIBNAME="`readlink $${LIBPATH}`"; \
		done; \
	done; \
 \
	echo -n

#
# Copy the full external toolchain sysroot directory to the staging
# dir. The operation of this function is rendered a little bit
# complicated by the support for multilib toolchains.
#
# We start by copying etc, lib, sbin and usr from the sysroot of the
# selected architecture variant (as pointed by ARCH_SYSROOT_DIR). This
# allows to import into the staging directory the C library and
# companion libraries for the correct architecture variant. We
# explictly only copy etc, lib, sbin and usr since other directories
# might exist for other architecture variants (on Codesourcery
# toolchain, the sysroot for the default architecture variant contains
# the armv4t and thumb2 subdirectories, which are the sysroot for the
# corresponding architecture variants), and we don't want to import
# them.
#
# Then, if the selected architecture variant is not the default one
# (i.e, if SYSROOT_DIR != ARCH_SYSROOT_DIR), then we :
#
#  * Import the header files from the default architecture
#    variant. Header files are typically shared between the sysroots
#    for the different architecture variants. If we use the
#    non-default one, header files were not copied by the previous
#    step, so we copy them here from the sysroot of the default
#    architecture variant.
#
#  * Create a symbolic link that matches the name of the subdirectory
#    for the architecture variant in the original sysroot. This is
#    required as the compiler will by default look in
#    sysroot_dir/arch_variant/ for libraries and headers, when the
#    non-default architecture variant is used. Without this, the
#    compiler fails to find libraries and headers.
#
# Some toolchains (i.e Linaro binary toolchains) store support
# libraries (libstdc++, libgcc_s) outside of the sysroot, so we simply
# copy all the libraries from the "support lib directory" into our
# sysroot.
#
# Note that the 'locale' directories are not copied. They are huge
# (400+MB) in CodeSourcery toolchains, and they are not really useful.
#
# $1: main sysroot directory of the toolchain
# $2: arch specific sysroot directory of the toolchain
# $3: arch specific subdirectory in the sysroot
# $4: directory of libraries ('lib' or 'lib64')
# $5: support lib directories (for toolchains storing libgcc_s,
#     libstdc++ and other gcc support libraries outside of the
#     sysroot)
copy_toolchain_sysroot = \
	SYSROOT_DIR="$(strip $1)"; \
	ARCH_SYSROOT_DIR="$(strip $2)"; \
	ARCH_SUBDIR="$(strip $3)"; \
	ARCH_LIB_DIR="$(strip $4)" ; \
	SUPPORT_LIB_DIR="$(strip $5)" ; \
	for i in etc $${ARCH_LIB_DIR} sbin usr ; do \
		if [ -d $${ARCH_SYSROOT_DIR}/$$i ] ; then \
			rsync -au --chmod=Du+w --exclude 'usr/lib/locale' $${ARCH_SYSROOT_DIR}/$$i $(STAGING_DIR)/ ; \
		fi ; \
	done ; \
	if [ `readlink -f $${SYSROOT_DIR}` != `readlink -f $${ARCH_SYSROOT_DIR}` ] ; then \
		if [ ! -d $${ARCH_SYSROOT_DIR}/usr/include ] ; then \
			cp -a $${SYSROOT_DIR}/usr/include $(STAGING_DIR)/usr ; \
		fi ; \
		mkdir -p `dirname $(STAGING_DIR)/$${ARCH_SUBDIR}` ; \
		relpath="./" ; \
		nbslashs=`echo -n $${ARCH_SUBDIR} | sed 's%[^/]%%g' | wc -c` ; \
		for slash in `seq 1 $${nbslashs}` ; do \
			relpath=$${relpath}"../" ; \
		done ; \
		ln -s $${relpath} $(STAGING_DIR)/$${ARCH_SUBDIR} ; \
		echo "Symlinking $(STAGING_DIR)/$${ARCH_SUBDIR} -> $${relpath}" ; \
	fi ; \
	if test -n "$${SUPPORT_LIB_DIR}" ; then \
		cp -a $${SUPPORT_LIB_DIR}/* $(STAGING_DIR)/lib/ ; \
	fi ; \
	find $(STAGING_DIR) -type d | xargs chmod 755

#
# Create lib64 -> lib and usr/lib64 -> usr/lib symbolic links in the
# target and staging directories. This is needed for some 64 bits
# toolchains such as the Crosstool-NG toolchains, for which the path
# to the dynamic loader and other libraries is /lib64, but the
# libraries are stored in /lib.
#
create_lib64_symlinks = \
	(cd $(TARGET_DIR) ;      ln -s lib lib64) ; \
	(cd $(TARGET_DIR)/usr ;  ln -s lib lib64) ; \
	(cd $(STAGING_DIR) ;     ln -s lib lib64) ; \
	(cd $(STAGING_DIR)/usr ; ln -s lib lib64)

#
# Check the availability of a particular glibc feature. We assume that
# all Buildroot toolchain options are supported by glibc, so we just
# check that they are enabled.
#
# $1: Buildroot option name
# $2: feature description
#
check_glibc_feature = \
	if [ x$($(1)) != x"y" ] ; then \
		echo "$(2) available in C library, please enable $(1)" ; \
		exit 1 ; \
	fi

#
# Check the correctness of a glibc external toolchain configuration.
#  1. Check that the C library selected in Buildroot matches the one
#     of the external toolchain
#  2. Check that all the C library-related features are enabled in the
#     config, since glibc always supports all of them
#
# $1: sysroot directory
#
check_glibc = \
	SYSROOT_DIR="$(strip $1)"; \
	if ! test -f $${SYSROOT_DIR}/lib/ld-linux*.so.* -o -f $${SYSROOT_DIR}/lib/ld.so.* ; then \
		echo "Incorrect selection of the C library"; \
		exit -1; \
	fi; \
	$(call check_glibc_feature,BR2_LARGEFILE,Large file support) ;\
	$(call check_glibc_feature,BR2_INET_IPV6,IPv6 support) ;\
	$(call check_glibc_feature,BR2_INET_RPC,RPC support) ;\
	$(call check_glibc_feature,BR2_ENABLE_LOCALE,Locale support) ;\
	$(call check_glibc_feature,BR2_USE_MMU,MMU support) ;\
	$(call check_glibc_feature,BR2_USE_WCHAR,Wide char support)

#
# Check the conformity of Buildroot configuration with regard to the
# uClibc configuration of the external toolchain, for a particular
# feature.
#
# $1: uClibc macro name
# $2: Buildroot option name
# $3: uClibc config file
# $4: feature description
#
check_uclibc_feature = \
	IS_IN_LIBC=`grep -q "\#define $(1) 1" $(3) && echo y` ; \
	if [ x$($(2)) != x"y" -a x$${IS_IN_LIBC} = x"y" ] ; then \
		echo "$(4) available in C library, please enable $(2)" ; \
		exit 1 ; \
	fi ; \
	if [ x$($(2)) = x"y" -a x$${IS_IN_LIBC} != x"y" ] ; then \
		echo "$(4) not available in C library, please disable $(2)" ; \
		exit 1 ; \
	fi

#
# Check the correctness of a uclibc external toolchain configuration
#  1. Check that the C library selected in Buildroot matches the one
#     of the external toolchain
#  2. Check that the features enabled in the Buildroot configuration
#     match the features available in the uClibc of the external
#     toolchain
#
# $1: sysroot directory
#
check_uclibc = \
	SYSROOT_DIR="$(strip $1)"; \
	if ! test -f $${SYSROOT_DIR}/usr/include/bits/uClibc_config.h ; then \
		echo "Incorrect selection of the C library"; \
		exit -1; \
	fi; \
	UCLIBC_CONFIG_FILE=$${SYSROOT_DIR}/usr/include/bits/uClibc_config.h ; \
	$(call check_uclibc_feature,__ARCH_USE_MMU__,BR2_USE_MMU,$${UCLIBC_CONFIG_FILE},MMU support) ;\
	$(call check_uclibc_feature,__UCLIBC_HAS_LFS__,BR2_LARGEFILE,$${UCLIBC_CONFIG_FILE},Large file support) ;\
	$(call check_uclibc_feature,__UCLIBC_HAS_IPV6__,BR2_INET_IPV6,$${UCLIBC_CONFIG_FILE},IPv6 support) ;\
	$(call check_uclibc_feature,__UCLIBC_HAS_RPC__,BR2_INET_RPC,$${UCLIBC_CONFIG_FILE},RPC support) ;\
	$(call check_uclibc_feature,__UCLIBC_HAS_LOCALE__,BR2_ENABLE_LOCALE,$${UCLIBC_CONFIG_FILE},Locale support) ;\
	$(call check_uclibc_feature,__UCLIBC_HAS_WCHAR__,BR2_USE_WCHAR,$${UCLIBC_CONFIG_FILE},Wide char support) ;\
	$(call check_uclibc_feature,__UCLIBC_HAS_THREADS__,BR2_TOOLCHAIN_HAS_THREADS,$${UCLIBC_CONFIG_FILE},Thread support) ;\
	$(call check_uclibc_feature,__PTHREADS_DEBUG_SUPPORT__,BR2_TOOLCHAIN_HAS_THREADS_DEBUG,$${UCLIBC_CONFIG_FILE},Thread debugging support)

#
# Check that the Buildroot configuration of the ABI matches the
# configuration of the external toolchain.
#
# $1: cross-gcc path
#
check_arm_abi = \
	__CROSS_CC=$(strip $1) ; \
	EXT_TOOLCHAIN_TARGET=`LANG=C $${__CROSS_CC} -v 2>&1 | grep ^Target | cut -f2 -d ' '` ; \
	if echo $${EXT_TOOLCHAIN_TARGET} | grep -q 'eabi$$' ; then \
		EXT_TOOLCHAIN_ABI="eabi" ; \
	else \
		EXT_TOOLCHAIN_ABI="oabi" ; \
	fi ; \
	if [ x$(BR2_ARM_OABI) = x"y" -a $${EXT_TOOLCHAIN_ABI} = "eabi" ] ; then \
		echo "Incorrect ABI setting" ; \
		exit 1 ; \
	fi ; \
	if [ x$(BR2_ARM_EABI) = x"y" -a $${EXT_TOOLCHAIN_ABI} = "oabi" ] ; then \
		echo "Incorrect ABI setting" ; \
		exit 1 ; \
	fi

#
# Check that the external toolchain supports C++
#
# $1: cross-g++ path
#
check_cplusplus = \
	__CROSS_CXX=$(strip $1) ; \
	$${__CROSS_CXX} -v > /dev/null 2>&1 ; \
	if test $$? -ne 0 ; then \
		echo "C++ support is selected but is not available in external toolchain" ; \
		exit 1 ; \
	fi

#
# Check that the cross-compiler given in the configuration exists
#
# $1: cross-gcc path
#
check_cross_compiler_exists = \
	__CROSS_CC=$(strip $1) ; \
	$${__CROSS_CC} -v > /dev/null 2>&1 ; \
	if test $$? -ne 0 ; then \
		echo "Cannot execute cross-compiler '$${__CROSS_CC}'" ; \
		exit 1 ; \
	fi
