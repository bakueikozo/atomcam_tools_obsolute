#!/bin/sh

HACK_INI=/media/mmc/hack.ini
STORAGE_SDCARD_REMOVE=$(awk -F "=" '/STORAGE_SDCARD_REMOVE *=/ {print $2}' $HACK_INI)
STORAGE_SDCARD_REMOVE_DAYS=$(awk -F "=" '/STORAGE_SDCARD_REMOVE_DAYS *=/ {print $2}' $HACK_INI)

STORAGE_CIFS_REMOVE=$(awk -F "=" '/STORAGE_CIFS_REMOVE *=/ {print $2}' $HACK_INI)
STORAGE_CIFS_REMOVE_DAYS=$(awk -F "=" '/STORAGE_CIFS_REMOVE_DAYS *=/ {print $2}' $HACK_INI)

STORAGE_SDCARD=$(awk -F "=" '/STORAGE_SDCARD *=/ {print $2}' $HACK_INI)
STORAGE_CIFS=$(awk -F "=" '/STORAGE_CIFS *=/ {print $2}' $HACK_INI)
STORAGE_CIFSSERVER=$(awk -F "=" '/STORAGE_CIFSSERVER *=/ {gsub(/\/$/, "", $2); print $2}' $HACK_INI)
STORAGE_CIFSUSER=$(awk -F "=" '/STORAGE_CIFSUSER *=/ {print $2}' $HACK_INI)
STORAGE_CIFSPASSWD=$(awk -F "=" '/STORAGE_CIFSPASSWD *=/ {print $2}' $HACK_INI)
HOSTNAME=`hostname`

if [ "$STORAGE_SDCARD_REMOVE" = "on" ] && [ "$STORAGE_SDCARD_REMOVE_DAYS" != "" ]; then
  if [ "$STORAGE_SDCARD" = "on" ]; then
    find /media/mmc/alarm_record /media/mmc/record /media/mmc/time_lapse -depth -type f -mtime $STORAGE_SDCARD_REMOVE_DAYS -delete
    find /media/mmc/alarm_record /media/mmc/record /media/mmc/time_lapse -depth -type d -empty -delete
  fi
fi

if [ "$STORAGE_CIFS_REMOVE" = "on" ] && [ "$STORAGE_CIFS_REMOVE_DAYS" != "" ]; then
  if [ "$STORAGE_CIFS" = "on" ] && [ "$STORAGE_CIFSSERVER" != "" ]; then
    if ! mount | grep "$STORAGE_CIFSSERVER" > /dev/null ; then
      mount -t cifs -ousername=$STORAGE_CIFSUSER,password=$STORAGE_CIFSPASSWD,vers=3.0 $STORAGE_CIFSSERVER /atom/mnt
    fi
    if [ $? = 0 ]; then
      find /atom/mnt/$HOSTNAME -depth -type f -mtime +$STORAGE_CIFS_REMOVE_DAYS -delete
      find /atom/mnt/$HOSTNAME -depth -type d -empty -delete
    fi
  fi
fi
