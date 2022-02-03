#!/bin/sh

HACK_INI=/media/mmc/hack.ini
RTSP_AUDIO=$(awk -F "=" '/RTSP_AUDIO *=/ {print $2}' $HACK_INI)

if [ "$1" = "on" ]; then
  kill `pidof v4l2rtspserver`
  touch /tmp/video_rtsp
  if [ "$RTSP_AUDIO" = "on" ] ; then
    touch /tmp/audio_rtsp
  else
    rm -rf /tmp/audio_rtsp
  fi
  (
    while ! ifconfig wlan0 | grep 'inet addr' > /dev/null
    do
      sleep 1
    done
     [ "`pidof v4l2rtspserver`" = "" ] && /usr/bin/v4l2rtspserver -C 1 -a S16_LE -l 0 /dev/video1,hw:Loopback,0 > /tmp/log/rtspserver.log 2>&1 &
  ) &
fi

if [ "$1" = "off" ]; then
  kill `pidof v4l2rtspserver`
  rm -f /tmp/video_rtsp /tmp/audio_rtsp
fi

exit 0
