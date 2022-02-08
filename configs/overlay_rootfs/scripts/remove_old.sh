#!/bin/sh

HACK_INI=/media/mmc/hack.ini
STORAGE_SDCARD_REMOVE=$(awk -F "=" '/STORAGE_SDCARD_REMOVE *=/ {print $2}' $HACK_INI)
STORAGE_SDCARD_REMOVE_DAYS=$(awk -F "=" '/STORAGE_SDCARD_REMOVE_DAYS *=/ {print $2}' $HACK_INI)

STORAGE_CIFS_REMOVE=$(awk -F "=" '/STORAGE_CIFS_REMOVE *=/ {print $2}' $HACK_INI)
STORAGE_CIFS_REMOVE_DAYS=$(awk -F "=" '/STORAGE_CIFS_REMOVE_DAYS *=/ {print $2}' $HACK_INI)

STORAGE_SDCARD=$(awk -F "=" '/STORAGE_SDCARD *=/ {print $2}' $HACK_INI)
HOSTNAME=`hostname`

if [ "$STORAGE_SDCARD_REMOVE" = "on" ] && [ "$STORAGE_SDCARD_REMOVE_DAYS" != "" ]; then
  if [ "$STORAGE_SDCARD" = "on" ]; then
    find /media/mmc/alarm_record /media/mmc/record /media/mmc/time_lapse -depth -type f -mtime $STORAGE_SDCARD_REMOVE_DAYS -delete
    find /media/mmc/alarm_record /media/mmc/record /media/mmc/time_lapse -depth -type d -empty -delete
  fi
fi

if [ "$STORAGE_CIFS_REMOVE" = "on" ] && [ "$STORAGE_CIFS_REMOVE_DAYS" != "" ]; then
  if /tmp/system/bin/mount_cifs ; then
    find /atom/mnt/$HOSTNAME -depth -type f -mtime +$STORAGE_CIFS_REMOVE_DAYS -delete
    find /atom/mnt/$HOSTNAME -depth -type d -empty -delete
  fi
fi
