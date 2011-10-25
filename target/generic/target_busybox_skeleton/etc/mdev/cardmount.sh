#!/bin/sh
TOP_DIR=/media
SUB_NAME=card$(echo $1 | cut -b 8,9,10)
DIR_NAME=${TOP_DIR}/${SUB_NAME}
MOUNTED=$(mount |grep $1) 

if [ "${MOUNTED}" = "" ];then
mkdir -p $DIR_NAME
mount -t vfat /dev/$1 ${DIR_NAME}
MOUNTED=$(mount |grep $1)
if [ "${MOUNTED}" = "" ];then
mount -t ext2 /dev/$1 ${DIR_NAME} 
MOUNTED=$(mount |grep $1)
if [ "${MOUNTED}" = "" ];then
FUSENODE=$(ls /dev |grep fuse)
if [ "${FUSENODE}" = "" ];then
mknod /dev/fuse c 10 229
fi
ntfs-3g /dev/$1 ${DIR_NAME}
fi
fi
echo "new disk $1 mount to ${DIR_NAME}" >> /log
kill -SIGURG -1
fi
