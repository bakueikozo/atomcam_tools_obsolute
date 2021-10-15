#!/bin/sh

export PATH=/system/bin:/bin:/sbin:/usr/bin:/usr/sbin

/system/bin/hl_client &

if [ "$1" = "on" ]; then
  LD_PRELOAD=/media/mmc/modules/libcallback.so /system/bin/iCamera_app >> /tmp/log/atom.log 2>&1 &
else
  /system/bin/iCamera_app >> /tmp/log/atom.log 2>&1 &
fi
