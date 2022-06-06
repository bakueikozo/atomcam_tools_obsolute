#!/bin/sh

[ -f /tmp/healthcheck.lock ] && exit 0
touch /tmp/healthcheck.lock

RES=1
[ -f /tmp/router_address ] && ROUTER=`cat /tmp/router_address`
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
  retry_count="0 0"
  [ -f /tmp/healthcheck.retry_count ] && retry_count=`cat /tmp/healthcheck.retry_count`
  retry=${retry_count% *}
  error=${retry_count#* }
  let retry++
  echo $(date +"%Y/%m/%d %H:%M:%S : retry : ") $retry >> /media/mmc/healthcheck.log
  ifconfig | grep 'Link encap:' | grep -v Loopback || retry=4
  if [ $retry -ge 3 ] ; then
    retry=0
    let error++
    if [ $error -ge 10 ] ; then
      echo $(date +"%Y/%m/%d %H:%M:%S : retry error -> reboot") >> /media/mmc/healthcheck.log
      echo router=$ROUTER >> /media/mmc/healthcheck.log
      [ "$ROUTER" != "" ] && ping -c 1 $ROUTER >> /media/mmc/healthcheck.log 2>&1
      ping -c 1 8.8.8.8 >> /media/mmc/healthcheck.log 2>&1
      ifconfig >> /media/mmc/healthcheck.log 2>&1
      df -k >> /media/mmc/healthcheck.log
      free >> /media/mmc/healthcheck.log
      killall -SIGUSR2 iCamera_app
      sync
      sync
      sync
      reboot
    fi
    echo $(date +"%Y/%m/%d %H:%M:%S : Network restart : error : ") $error >> /media/mmc/healthcheck.log

    [ -x /media/mmc/network_init.sh ] && /media/mmc/network_init.sh restart >> /media/mmc/healthcheck.log 2>&1
    [ "$?" = "100" ] && error=0
    if ifconfig | grep wlan0 > /dev/null 2>&1 ; then
      ifconfig wlan0 down
      ifconfig wlan0 up
      killall -USR1 udhcpc
      ps | grep -v grep | grep udhcpc || udhcpc -i wlan0 -x hostname:ATOM -p /var/run/udhcpc.pid -b >> /media/mmc/healthcheck.log 2>&1
    fi
  fi
  echo $retry $error > /tmp/healthcheck.retry_count
else
  rm -f /tmp/healthcheck.retry_count
fi
rm -f /tmp/healthcheck.lock
