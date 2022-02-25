#!/bin/sh

if [ "$1" = "off" ]; then
  /scripts/cmd audio off > /dev/null
  /scripts/cmd video off > /dev/null
  kill `pidof v4l2rtspserver` > /dev/null 2>&1
  exit 0
fi

HACK_INI=/tmp/hack.ini
RTSP_AUDIO=$(awk -F "=" '/RTSP_AUDIO *=/ {print $2}' $HACK_INI)
RTSPSERVER=$(awk -F "=" '/RTSPSERVER *=/ {print $2}' $HACK_INI)
if [ "$1" = "on" -o "$RTSPSERVER" = "on" ]; then
  /scripts/cmd video on > /dev/null
  /scripts/cmd audio on > /dev/null
  if ! pidof v4l2rtspserver > /dev/null ; then
    while netstat -ltn 2> /dev/null | grep :8554; do
      sleep 0.5
    done
    /usr/bin/v4l2rtspserver -C 1 -a S16_LE -l 0 /dev/video1,hw:Loopback,0 >> /tmp/log/rtspserver.log 2>&1 &
  fi
  if [ "$RTSP_AUDIO" != "on" ] ; then
    /scripts/cmd audio off > /dev/null
  fi
  while [ "`pidof v4l2rtspserver`" = "" ]; do
    sleep 0.5
  done
fi

exit 0
