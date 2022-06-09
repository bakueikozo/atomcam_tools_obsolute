#!/bin/sh

if [ "$1" = "off" -o "$1" = "restart" ]; then
  /scripts/cmd audio off > /dev/null
  /scripts/cmd video 0 off > /dev/null
  #/scripts/cmd video 1 off > /dev/null
  kill `pidof v4l2rtspserver` > /dev/null 2>&1
  [ "$1" = "off" ] && exit 0
  while pidof v4l2rtspserver > /dev/null ; do
    sleep 0.5
  done
  echo `date +"%Y/%m/%d %H:%M:%S"` ": v4l2rtspserver stop"
fi

HACK_INI=/tmp/hack.ini
RTSP_OVER_HTTP=$(awk -F "=" '/RTSP_OVER_HTTP *=/ {print $2}' $HACK_INI)
RTSP_AUDIO=$(awk -F "=" '/RTSP_AUDIO *=/ {print $2}' $HACK_INI)
RTSPSERVER=$(awk -F "=" '/RTSPSERVER *=/ {print $2}' $HACK_INI)

if [ "$1" = "watchdog" ]; then
  [ "$RTSPSERVER" = "on" ] || exit 0
  pidof v4l2rtspserver > /dev/null && exit 0
fi

if [ "$1" = "on" -o "$1" = "restart" -o "$1" = "watchdog" -o "$RTSPSERVER" = "on" ]; then
  /scripts/cmd video 0 on > /dev/null
  #/scripts/cmd video 1 on > /dev/null
  /scripts/cmd audio on > /dev/null
  if ! pidof v4l2rtspserver > /dev/null ; then
    while netstat -ltn 2> /dev/null | egrep ":(8554|8080)"; do
      sleep 0.5
    done
    echo `date +"%Y/%m/%d %H:%M:%S"` ": v4l2rtspserever start"
    if [ "$RTSP_OVER_HTTP" = "on" ] ; then
      /usr/bin/v4l2rtspserver -p 8080 -C 1 -a S16_LE -l 0 /dev/video0,hw:Loopback,0 >> /tmp/log/rtspserver.log 2>&1 &
    else
      /usr/bin/v4l2rtspserver -C 1 -a S16_LE -l 0 /dev/video0,hw:Loopback,0 >> /tmp/log/rtspserver.log 2>&1 &
    fi
  fi
  if [ "$RTSP_AUDIO" != "on" ] ; then
    /scripts/cmd audio off > /dev/null
  fi
  while [ "`pidof v4l2rtspserver`" = "" ]; do
    sleep 0.5
  done
fi

exit 0
