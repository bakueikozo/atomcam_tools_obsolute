#!/bin/sh

if [ "$1" = "on" ]; then
  (
    while ! ifconfig wlan0 | grep 'inet addr'
    do
      sleep 1
    done
     [ "`pidof v4l2rtspserver`" = "" ] && /usr/bin/v4l2rtspserver -C 1 -a S16_LE -l 0 /dev/video1,hw:Loopback,0 > /tmp/log/rtspserver.log 2>&1 &
  ) &
fi

if [ "$1" = "off" ]; then
  kill `pidof v4l2rtspserver`
fi
