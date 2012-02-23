#!/bin/sh
TOP_DIR=/media
SUB_NAME=card$(echo $1 | cut -b 8,9,10)
DIR_NAME=${TOP_DIR}/${SUB_NAME}

if [ x$ACTION = "x" ] || [ ${ACTION} == "add" ] ; then
    mkdir -p $DIR_NAME
    mount -t vfat -o utf8 /dev/$1 ${DIR_NAME}
    MOUNTED=$(mount |grep $1)
    if [ "${MOUNTED}" = "" ] ; then
        mount -t ext2 -o utf8 /dev/$1 ${DIR_NAME} 
        MOUNTED=$(mount |grep $1)
        if [ "${MOUNTED}" = "" ] ; then
            ntfs-3g -o utf8 /dev/$1 ${DIR_NAME}
        fi
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
