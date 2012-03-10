#!/bin/sh
TOP_DIR=/media
SUB_NAME=card$(echo $1 | cut -b 8,9,10)
DIR_NAME=${TOP_DIR}/${SUB_NAME}
SYSTEM_NTFS=$(fdisk -l|grep NTFS|grep $1)
SYSTEM_FAT32=$(fdisk -l|grep FAT32|grep $1)

if [ x$ACTION = "x" ] || [ ${ACTION} == "add" ] ; then
    mkdir -p $DIR_NAME
    if [ "${SYSTEM_NTFS}" = "NTFS" ]; then
        ntfs-3g -o utf8 /dev/$1 ${DIR_NAME}
    elif [ "${SYSTEM_FAT32}" = "FAT32" ] ; then
        mount -t vfat -o utf8 /dev/$1 ${DIR_NAME} 
    else
        mount /dev/$1 ${DIR_NAME}
    fi
    echo "new disk $1 mount to ${DIR_NAME}" >> /log
    exit 0
fi

if [ $ACTION = "remove" ] ; then
    umount -l ${DIR_NAME}
    rm -rf ${DIR_NAME}
    echo "disk  ${DIR_NAME} removed" >> /log
    exit 0
fi

echo "$0: $ACTION=ACTION not recognized" > /dev/console
env > /dev/console
