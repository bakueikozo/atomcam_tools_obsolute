#!/bin/sh

HACK_INI=/tmp/hack.ini
echo `TZ=JST-9 date +"%Y/%m/%d %H:%M:%S"` ": Reboot & Start watchdog" >> /media/mmc/atomhack.log

count=0
wifi_error=0
while sleep 20 ; do
  let count++

  # wifi
  router=`ip route | awk '/default/ { print $3 }'`
  if ping -c 1 $router > /dev/null ; then
    wifi_error=0

    # process check 1min interval
    if [ `expr $count % 3` -eq 0 ]; then
      /scripts/lighttpd.sh watchdog >> /media/mmc/atomhack.log
      /scripts/rtspserver.sh watchdog >> /media/mmc/atomhack.log

      HEALTHCHECK=$(awk -F "=" '/HEALTHCHECK *=/ {print $2}' $HACK_INI)
      HEALTHCHECK_PING_URL=$(awk -F "=" '/HEALTHCHECK_PING_URL *=/ {print $2}' $HACK_INI)
      [ "$HEALTHCHECK" == "on" ] && [ "$HEALTHCHECK_PING_URL" != "" ] && echo `TZ=JST-9 date +"%Y/%m/%d %H:%M:%S"` >> /media/mmc/healthcheck.log && curl -fsS -m 10 --retry 5 $HEALTHCHECK_PING_URL >> /media/mmc/healthcheck.log
    fi
  else
    let wifi_error++
  fi

  if [ $wifi_error -ge 3 ]; then
    echo "WiFi restart" >> /media/mmc/atomhack.log
    ifconfig wlan0 down
    ifconfig wlan0 up
    wifi_error=0
  fi

  # free memory
  if [ $count -ge 180 ]; then # 1H interval
    count=0
    TZ=JST-9 date >> /media/mmc/atomhack.log
    free >> /media/mmc/atomhack.log
  fi
done
