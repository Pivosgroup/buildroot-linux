#!/bin/sh
LOGFILE="/etc/mdev/amvdec.log"
MODPATH="/lib/modules"

#echo ${SUBSYSTEM} ${ACTION} ${MDEV} >> ${LOGFILE};

if [ ${SUBSYSTEM} == "platform" ]; then
    if [ ${ACTION} == "add" ]; then
#echo "add $1" >> ${LOGFILE};
        insmod ${MODPATH}/amvideo.ko;
        insmod ${MODPATH}/amvdec_util.ko;
        insmod ${MODPATH}/$1.ko;
    elif [ ${ACTION} == "remove" ]; then
#echo "remove $1" >> ${LOGFILE};
        rmmod $1;
        rmmod amvdec_util;
        rmmod amvideo;
    fi
fi