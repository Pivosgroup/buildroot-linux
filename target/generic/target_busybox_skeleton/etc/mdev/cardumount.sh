#!/bin/sh
TOP_DIR=/media
SUB_NAME=card$(echo $1 | cut -b 8,9,10)
DIR_NAME=${TOP_DIR}/${SUB_NAME} 
umount ${DIR_NAME} 
rm -rf ${DIR_NAME}
echo "disk  ${DIR_NAME} removed" >> /log
kill -SIGURG -1
