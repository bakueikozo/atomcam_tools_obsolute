#!/bin/sh
#
# for timelapse file
#

HACK_INI=/tmp/hack.ini
STORAGE_CIFS=$(awk -F "=" '/STORAGE_CIFS *=/ { gsub(/^\/*/, "", $2);print $2}' $HACK_INI)
STORAGE_CIFS_PATH=$(awk -F "=" '/STORAGE_CIFS_PATH *=/ { gsub(/^\/*/, "", $2);print $2}' $HACK_INI)
STORAGE_SDCARD=$(awk -F "=" '/STORAGE_SDCARD *=/ {print $2}' $HACK_INI)
WEBHOOK=$(awk -F "=" '/WEBHOOK *=/ {print $2}' $HACK_INI)
WEBHOOK_URL=$(awk -F "=" '/WEBHOOK_URL *=/ {print $2}' $HACK_INI)
WEBHOOK_TIMELAPSE_FINISH=$(awk -F "=" '/WEBHOOK_TIMELAPSE_FINISH *=/ {print $2}' $HACK_INI)
HOSTNAME=`hostname`

(
  if [ "$STORAGE_CIFS" = "on" -o "$STORAGE_CIFS" = "record" ] && /scripts/mount_cifs.sh ; then
    CIFSFILE="time_lapse/${1##*/}"
    OUTFILE="/atom/mnt/$HOSTNAME/$CIFSFILE"
    DIR_PATH=${OUTFILE%/*}
    mkdir -p $DIR_PATH
    cp $1 $OUTFILE
    STORAGE=", \"cifsFile\":\"${CIFSFILE}\""
  fi

  if [ "$STORAGE_SDCARD" = "on" -o "$STORAGE_SDCARD" = "record" ]; then
    STORAGE="${STORAGE}, \"sdcardFile\":\"${1##*media/mmc/}\""
  else
    echo "skip remove $1 (test)"
    # rm $1
    find /media/mmc/time_lapse -depth -type d -empty -delete
  fi
  if [ "$WEBHOOK" = "on" ] && [ "$WEBHOOK_URL" != "" ] && [ "$WEBHOOK_TIMELAPSE_FINISH" = "on" ]; then
    /usr/bin/curl -X POST -m 3 -H "Content-Type: application/json" -d "{\"type\":\"timelapseFinish\", \"device\":\"${HOSTNAME}\"${STORAGE}}" $WEBHOOK_URL > /dev/null 2>&1
  fi
) &
exit 0
