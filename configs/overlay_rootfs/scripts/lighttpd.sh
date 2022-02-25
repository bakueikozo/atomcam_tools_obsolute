#!/bin/sh

if [ "$1" = "off" ]; then
  kill `pidof lighttpd` > /dev/null 2>&1
  exit 0
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

if [ "$1" = "on" ]; then
  /usr/sbin/lighttpd -f /etc/lighttpd/lighttpd.conf
fi

if [ "$1" = "restart" ]; then
  kill `pidof lighttpd` > /dev/null 2>&1
  while netstat -ltn 2> /dev/null | grep :80 > /dev/null; do
    sleep 0.5
  done
  /usr/sbin/lighttpd -f /etc/lighttpd/lighttpd.conf
fi
