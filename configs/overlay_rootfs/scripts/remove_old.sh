#!/bin/sh

HACK_INI=/media/mmc/hack.ini
STORAGE_REMOVE=$(awk -F "=" '/STORAGE_REMOVE *=/ {print $2}' $HACK_INI)
STORAGE_REMOVE_DAYS=$(awk -F "=" '/STORAGE_REMOVE_DAYS *=/ {print $2}' $HACK_INI)

STORAGE_SDCARD=$(awk -F "=" '/STORAGE_SDCARD *=/ {print $2}' $HACK_INI)
STORAGE_CIFS=$(awk -F "=" '/STORAGE_CIFS *=/ {print $2}' $HACK_INI)
STORAGE_CIFSSERVER=$(awk -F "=" '/STORAGE_CIFSSERVER *=/ {gsub(/\/$/, "", $2); print $2}' $HACK_INI)
STORAGE_CIFSUSER=$(awk -F "=" '/STORAGE_CIFSUSER *=/ {print $2}' $HACK_INI)
STORAGE_CIFSPASSWD=$(awk -F "=" '/STORAGE_CIFSPASSWD *=/ {print $2}' $HACK_INI)
HOSTNAME=`hostname`

if [ "$STORAGE_REMOVE" = "on" ] && [ "$STORAGE_REMOVE_DAYS" != "" ]; then
  if [ "$STORAGE_SDCARD" = "on" ]; then
    find /media/mmc/alarm_record /media/mmc/record /media/mmc/time_lapse -depth -mtime $STORAGE_REMOVE_DAYS -exec busybox rm -rf {} +
  fi
  if [ "$STORAGE_CIFS" = "on" ] && [ "$STORAGE_CIFSSERVER" != "" ]; then
    if ! mount | grep "$STORAGE_CIFSSERVER" > /dev/null ; then
      mount -t cifs -ousername=$STORAGE_CIFSUSER,password=$STORAGE_CIFSPASSWD $STORAGE_CIFSSERVER /atom/mnt
    fi
    if [ $? = 0 ]; then
      find /atom/mnt/$HOSTNAME -depth -mtime +$STORAGE_REMOVE_DAYS -exec busybox rm -rf {} +
    fi
  fi
fi
