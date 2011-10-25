#!/bin/sh
TOP_DIR=/media
SUB_NAME=usb$(echo $1 | cut -b 3,4)
DIR_NAME=${TOP_DIR}/${SUB_NAME}
SYSTEM=$(fdisk -l|grep NTFS)

mkdir -p $DIR_NAME
if [ "${SYSTEM}" = "" ];then
mount -t vfat /dev/$1 ${DIR_NAME} 
else
ntfs-3g /dev/$1 ${DIR_NAME}
fi

echo "new disk $1 mount to ${DIR_NAME}" >> /log
kill -SIGURG -1
