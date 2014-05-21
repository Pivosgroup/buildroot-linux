#!/bin/sh
# If its still mounted, then dismount it
if grep -qs '/media/camera ' /proc/mounts; then
      umount /media/camera
fi
