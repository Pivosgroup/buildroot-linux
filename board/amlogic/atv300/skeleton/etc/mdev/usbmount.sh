#!/bin/sh
TOP_DIR=/media
SUB_NAME=usb$(echo $1 | cut -b 3,4)
DIR_NAME=${TOP_DIR}/${SUB_NAME}
SYSTEM=$(fdisk -l|grep NTFS|grep $1)


if [ ${ACTION} == "add" ]; 
then
    mkdir -p $DIR_NAME
    if [ "${SYSTEM}" = "" ];
    then
        mount -t vfat -o utf8 /dev/$1 ${DIR_NAME} 
    else
        ntfs-3g -o utf8 /dev/$1 ${DIR_NAME}
    fi
    echo "new disk $1 mount to ${DIR_NAME}" >> /log
else
    umount -l ${DIR_NAME} 
    rm -rf ${DIR_NAME}
    echo "disk  ${DIR_NAME} removed" >> /log
fi

