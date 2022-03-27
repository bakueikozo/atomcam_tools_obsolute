#!/bin/sh
#
# for alarm record file
#

[ "$1" != "" ] && [ -e $1 ] && FILE=$1
[ "$2" != "" ] && [ -e $2 ] && FILE=$2
if [ "$FILE" != "/tmp/alarm.jpg" ] && [ "$FILE" != "/tmp/alarm_record.mp4" ] ; then
  /bin/busybox ${0##*/} $*
  exit
fi

HACK_INI=/tmp/hack.ini
RECORDING_ALARM=$(awk -F "=" '/RECORDING_ALARM *=/ {print $2}' $HACK_INI)
RECORDING_LOCAL_SCHEDULE=$(awk -F "=" '/RECORDING_LOCAL_SCHEDULE *=/ {print $2}' $HACK_INI)
STORAGE_CIFS_PATH=$(awk -F "=" '/STORAGE_CIFS_PATH *=/ { gsub(/^\/*/, "", $2);print $2}' $HACK_INI)
STORAGE_SDCARD_PATH=$(awk -F "=" '/STORAGE_SDCARD_PATH *=/ { gsub(/^\/*/, "", $2);print $2}' $HACK_INI)
STORAGE_SDCARD=$(awk -F "=" '/STORAGE_SDCARD *=/ {print $2}' $HACK_INI)
WEBHOOK=$(awk -F "=" '/WEBHOOK *=/ {print $2}' $HACK_INI)
WEBHOOK_URL=$(awk -F "=" '/WEBHOOK_URL *=/ {print $2}' $HACK_INI)
WEBHOOK_ALERM_PICT=$(awk -F "=" '/WEBHOOK_ALERM_PICT *=/ {print $2}' $HACK_INI)
WEBHOOK_ALARM_PICT_FINISH=$(awk -F "=" '/WEBHOOK_ALARM_PICT_FINISH *=/ {print $2}' $HACK_INI)
WEBHOOK_ALERM_VIDEO=$(awk -F "=" '/WEBHOOK_ALERM_VIDEO *=/ {print $2}' $HACK_INI)
WEBHOOK_ALARM_VIDEO_FINISH=$(awk -F "=" '/WEBHOOK_ALARM_VIDEO_FINISH *=/ {print $2}' $HACK_INI)
HOSTNAME=`hostname`

if [ "$RECORDING_LOCAL_SCHEDULE" = "on" ]; then
  FMT=`TZ=JST-9 awk '
    BEGIN {
      FS = "=";
      INDEX=-1;
    }
    /\[index_/ {
      INDEX = $0;
      gsub(/^.*_/, "", INDEX);
      gsub(/\].*$/, "", INDEX);
      INDEX = INDEX + 0;
      ENABLE[INDEX] = 1;
    }
    /Rule=/ {
      day = strftime("%w");
      dayMask = $2;
      if(day == 0) day = 7;
      for(i = 0; i < day; i++) {
        dayMask = int(dayMask / 2);
      }
      if(dayMask % 2 == 0) ENABLE[INDEX] = 0;
    }
    /ContinueTime=/ {
      CONTINUETIME[INDEX] = $2;
    }
    /StartTime=/ {
      STARTTIME[INDEX] = $2;
    }
    /Status=/ {
      if($2 != 1) ENABLE[INDEX] = 0;
    }
    /DelFlags=/ {
      if($2 != 1) ENABLE[INDEX] = 0;
    }
    END {
      HOUR = strftime("%H");
      MINUTE = strftime("%M");
      NOW = HOUR * 60 + MINUTE;
      FLAG = 0;
      for(i = 1; i <= INDEX; i++) {
        if(NOW < STARTTIME[i]) ENABLE[i] = 0;
        if(NOW >= STARTTIME[i] + CONTINUETIME[i]) ENABLE[i] = 0;
        if(ENABLE[i]) FLAG = 1;
      }
      if(FLAG) print strftime("%Y%m%d_%H%M%S");
    }
  ' /media/mmc/local_schedule`
else
  FMT=`TZ=JST-9 date +"%Y%m%d_%H%M%S"`
fi

if [ "$WEBHOOK" = "on" ] && [ "$WEBHOOK_URL" != "" ]; then
  (
    if [ "$WEBHOOK_ALERM_PICT" = "on" ] && [ "$FILE" = "/tmp/alarm.jpg" ]; then
      LD_LIBRARY_PATH=/tmp/system/lib:/usr/lib /tmp/system/lib/ld.so.1 /tmp/system/bin/curl -X POST -m 3 -F "image=@$FILE" -F"type=image/jpeg" -F"device=${HOSTNAME}" $WEBHOOK_URL > /dev/null 2>&1
    fi
    if [ "$WEBHOOK_ALERM_VIDEO" = "on" ] && [ "$FILE" = "/tmp/alarm_record.mp4" ]; then
      LD_LIBRARY_PATH=/tmp/system/lib:/usr/lib /tmp/system/lib/ld.so.1 /tmp/system/bin/curl -X POST -m 3 -F "video=@$FILE" -F "type=video/mp4" -F"device=${HOSTNAME}" $WEBHOOK_URL > /dev/null 2>&1
    fi
  ) &
fi

if [ "$FMT" != "" ] && [ "$RECORDING_ALARM" = "on" ]; then
  TMPFILE="/tmp/`cat /proc/sys/kernel/random/uuid`"
  mv $FILE $TMPFILE
  (
    if /tmp/system/bin/mount_cifs ; then
      OUTFILE=`TZ=JST-9 date +"/mnt/$HOSTNAME/alarm_record/$STORAGE_CIFS_PATH.${FILE##*.}"`
      DIR_PATH=${OUTFILE%/*}
      mkdir -p $DIR_PATH
      cp $TMPFILE $OUTFILE
    fi

    if [ "$STORAGE_SDCARD" = "on" ]; then
      OUTFILE=`TZ=JST-9 date +"/media/mmc/alarm_record/$STORAGE_SDCARD_PATH.${FILE##*.}"`
      DIR_PATH=${OUTFILE%/*}
      mkdir -p $DIR_PATH
      /bin/busybox mv $TMPFILE $OUTFILE
    else
      /bin/busybox rm $TMPFILE
    fi

    if [ "$WEBHOOK" = "on" ] && [ "$WEBHOOK_URL" != "" ]; then
      if [ "$WEBHOOK_ALARM_PICT_FINISH" = "on" ] && [ "$FILE" = "/tmp/alarm.jpg" ]; then
        LD_LIBRARY_PATH=/tmp/system/lib:/usr/lib /tmp/system/lib/ld.so.1 /tmp/system/bin/curl -X POST -m 3 -H "Content-Type: application/json" -d "{\"type\":\"uploadPictureFinish\", \"device\":\"${HOSTNAME}\"}" $WEBHOOK_URL > /dev/null 2>&1
      fi
      if [ "$WEBHOOK_ALARM_VIDEO_FINISH" = "on" ] && [ "$FILE" = "/tmp/alarm_record.mp4" ]; then
        LD_LIBRARY_PATH=/tmp/system/lib:/usr/lib /tmp/system/lib/ld.so.1 /tmp/system/bin/curl -X POST -m 3 -H "Content-Type: application/json" -d "{\"type\":\"uploadVideoFinish\", \"device\":\"${HOSTNAME}\"}" $WEBHOOK_URL > /dev/null 2>&1
      fi
    fi
  ) &
else
  /bin/busybox rm $FILE
fi
exit 0
