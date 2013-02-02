#!/bin/sh
# if ACTION from udev is add then 
if [ "$ACTION" == "add" ] ; then
  # get device name from udev variable
  DRIVE=$DEVNAME
  # check mounted drive for this drive
  MOUNTSTRING=`fgrep $DRIVE /proc/mounts`
  MOUNTDEV=`echo $MOUNTSTRING | awk '{print $1}'`
  MOUNTPOINT=`echo $MOUNTSTRING | awk '{print $2}'`
  OUTDIR="$MOUNTPOINT/getlogs"
  OUTFILE="$OUTDIR/diag.txt"
  # check if drive is mounted
  if [ "$MOUNTDEV" == "$DRIVE" ]; then
    # check if output directory exists
    if [ -d "$OUTDIR" ]; then
       # copy xbmc logfiles to $OUTDIR
       cp /root/.xbmc/temp/xbmc*.log $OUTDIR
       # redirect stdout/stderror to $OUTFILE
       exec 1>$OUTFILE 2>&1
       # if runme exists in directory then run it
       # output results to $OUTDIR/runme.txt
       if [ -f "$OUTDIR/runme.sh" ]; then
          sh $OUTDIR/runme.sh $OUTDIR 1>$OUTDIR/runme.txt 2>&1
       fi
	
    else
       exit
     fi
  fi
fi
# run some diagnostics
echo free:-----------------
free
echo
echo iwconfig:-------------
iwconfig
echo
echo ifconfig:-------------
ifconfig
echo
echo df:-------------------
df
echo
echo ping:-----------------
ping -c 1 www.google.com
echo
echo lsmod:----------------
lsmod
echo

