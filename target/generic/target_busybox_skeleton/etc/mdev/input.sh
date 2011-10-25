#!/bin/sh
if [ ! -d /dev/input  ];then
echo make new dir /dev/input
mkdir -p /dev/input
fi
/bin/mv /dev/$1  /dev/input/
