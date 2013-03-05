#!/bin/sh
#Make Directory if it doesn't exist
if [ ! -d /media/camera ]; then
      /bin/mkdir /media/camera
fi
# If something already mounted, dismount it
if grep -qs '/media/camera ' /proc/mounts; then
      /bin/umount /media/camera
fi
# Mount the camera
/usr/bin/gphotofs /media/camera
