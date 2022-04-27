#!/bin/sh

[ ! -f /tmp/router_address ] && exit 0
[ -f /tmp/healthcheck.lock ] && exit 0
touch /tmp/healthcheck.lock

RES=1
ROUTER=`cat /tmp/router_address`
if [ "$ROUTER" = "" ]; then
  ip route | awk '/default/ { print $3 }' > /tmp/router_address
  ROUTER=`cat /tmp/router_address`
fi
[ "$ROUTER" != "" ] && ping -c 1 $ROUTER > /dev/null || RES=0

if [ $RES -eq 1 ] ; then
  HACK_INI=/tmp/hack.ini
  HEALTHCHECK=$(awk -F "=" '/HEALTHCHECK *=/ {print $2}' $HACK_INI)
  HEALTHCHECK_PING_URL=$(awk -F "=" '/HEALTHCHECK_PING_URL *=/ {print $2}' $HACK_INI)
  if [ "$HEALTHCHECK" = "on" -a "$HEALTHCHECK_PING_URL" != "" ] ; then
    RES=`curl --ipv4 --max-time 5 --retry 3 --retry-delay 1 --location --silent --show-error --output /dev/null --write-out "%{http_code}" $HEALTHCHECK_PING_URL`
    echo $(date +"%Y/%m/%d %H:%M:%S : ") $RES >> /media/mmc/healthcheck.log
  fi
fi

if [ $RES -eq 0 ] ; then
  retry=0
  [ -f /tmp/healthcheck.retry_count ] && retry=`cat /tmp/healthcheck.retry_count`
  let retry++
  echo $(date +"%Y/%m/%d %H:%M:%S : retry : ") $retry >> /media/mmc/healthcheck.log
  if [ $retry -ge 3 ] ; then
    echo $(date +"%Y/%m/%d %H:%M:%S : WiFi restart") >> /media/mmc/healthcheck.log
    ifconfig wlan0 down
    ifconfig wlan0 up
    killall -USR1 udhcpc || udhcpc -i wlan0 -p /var/run/udhcpc.pid
    rm -f /tmp/healthcheck.retry_count
  else
    echo $retry > /tmp/healthcheck.retry_count
  fi
else
  rm -f /tmp/healthcheck.retry_count
fi
rm -f /tmp/healthcheck.lock
