#!/bin/sh
TOP_DIR=/media
SUB_NAME=usb$(echo $1 | cut -b 3,4)
DIR_NAME=${TOP_DIR}/${SUB_NAME} 
umount ${DIR_NAME} 
rm -rf ${DIR_NAME}
echo "disk  ${DIR_NAME} removed" >> /log
kill -SIGURG -1
