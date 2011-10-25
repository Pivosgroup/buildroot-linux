#!/bin/sh

file=$1
var=$(svn info |grep Revision |wc -l)
if [ $var = 0 ] ;then
	SVN_VER=0
else
	SVN_VER=`svn info |grep Revision |cut -b 11-15`
fi
echo "#define SVN_VERSION $SVN_VER" > $file
#rm src/control/include/version.h
#mv version.h ./src/control/include/

