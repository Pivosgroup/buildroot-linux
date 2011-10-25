#!/bin/bash

#PREBUILT=$NDK_ROOT/build/prebuilt/linux-x86/arm-eabi-4.4.0
#PLATFORM=$NDK_ROOT/build/platforms/android-8/arch-arm
TARGET_CC=/opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc
CC=/opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-
PREFIX=/home/gongping/Project_ARMLinux/arm_ref/build_rootfs/7266m_8626m_demo/build/target/usr
STAGING_DIR=/home/gongping/Project_ARMLinux/arm_ref/build_rootfs/7266m_8626m_demo/build/build/libplayer/amadec/../../../staging

./configure \
	--cc=$TARGET_CC \
	--arch=arm \
	--enable-cross-compile --cross-prefix=$CC \
	--prefix=$PREFIX \
	--incdir=$STAGING_DIR/usr/include \
	--extra-ldflags=L$PREFIX/lib \
	--disable-static --enable-shared \
	--disable-ffmpeg --disable-ffplay --disable-ffserver \
	--disable-doc \
	--disable-mpegaudio-hp \
	--disable-encoders \
	--disable-decoder=h264 \
	--disable-muxers \
	--disable-filters \
	--disable-altivec \
	--disable-amd3dnow \
	--disable-amd3dnowext \
	--disable-mmx --disable-mmx2 --disable-sse --disable-ssse3 \
	--disable-armv5te --disable-armv6 --disable-armv6t2 --disable-armvfp \
	--disable-iwmmxt --disable-mmi --disable-vis --disable-yasm \
	--enable-pic

find . -name Makefile -print0 | xargs -0 sed -i '/..\/subdir.mak/d'
find . -name Makefile -print0 | xargs -0 sed -i '/..\/config.mak/d'
sed -i 's/restrict restrict/restrict/' config.h

