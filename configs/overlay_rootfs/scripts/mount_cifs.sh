#!/bin/sh

HACK_INI=/media/mmc/hack.ini
STORAGE_CIFS=$(awk -F "=" '/STORAGE_CIFS *=/ {print $2}' $HACK_INI)
STORAGE_CIFSSERVER=$(awk -F "=" '/STORAGE_CIFSSERVER *=/ {gsub(/\/$/, "", $2); print $2}' $HACK_INI)
STORAGE_CIFSUSER=$(awk -F "=" '/STORAGE_CIFSUSER *=/ {print $2}' $HACK_INI)
STORAGE_CIFSPASSWD=$(awk -F "=" '/STORAGE_CIFSPASSWD *=/ {print $2}' $HACK_INI)

if [ "$STORAGE_CIFS" = "on" ] && [ "$STORAGE_CIFSSERVER" != "" ]; then
  mount | grep "$STORAGE_CIFSSERVER" > /dev/null && exit
  for VER in 3.0 2.1 2.0
  do
    LD_LIBRARY_PATH=/tmp/system/lib:/usr/lib:/usr/lib/samba /tmp/system/lib/ld.so.1 /tmp/system/bin/busybox mount -t cifs -ousername=$STORAGE_CIFSUSER,password=$STORAGE_CIFSPASSWD,vers=$VER $STORAGE_CIFSSERVER /mnt && exit
  done
fi
exit -1
