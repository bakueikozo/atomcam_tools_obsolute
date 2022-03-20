#!/bin/sh

if [ "$1" = "off" ]; then
  kill `pidof lighttpd` > /dev/null 2>&1
  echo `TZ=JST-9 date +"%Y/%m/%d %H:%M:%S"` ": lighttpd stop"
  exit 0
fi

if [ "$1" = "watchdog" ]; then
  pidof lighttpd > /dev/null && exit 0
fi

HACK_INI=/tmp/hack.ini
DIGEST=$(awk -F "=" '/DIGEST *=/ {print $2}' $HACK_INI)
if [ "$DIGEST" != "" ]; then
  echo $DIGEST > /etc/lighttpd/user.digest
  echo 'server.modules += ( "mod_auth" )' > /etc/lighttpd/auth.conf
else
  echo $DIGEST > /etc/lighttpd/user.digest
  echo '#server.modules += ( "mod_auth" )' > /etc/lighttpd/auth.conf
fi

if [ "$1" = "restart" ]; then
  kill `pidof lighttpd` > /dev/null 2>&1
  while netstat -ltn 2> /dev/null | grep ':80 ' > /dev/null; do
    sleep 0.5
  done
fi

echo `TZ=JST-9 date +"%Y/%m/%d %H:%M:%S"` ": lighttpd start"
/usr/sbin/lighttpd -f /etc/lighttpd/lighttpd.conf
