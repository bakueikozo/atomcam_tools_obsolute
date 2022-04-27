#!/bin/sh
#
# for continuous recording file
#

if [ "${1%/*.mp4}" != "/tmp" ] || [ "${2##/media/mmc/record/*.mp4}" != "" ] ; then
  /bin/busybox ${0##*/} $*
  exit
fi

HACK_INI=/tmp/hack.ini
RECORDING_LOCAL_SCHEDULE=$(awk -F "=" '/RECORDING_LOCAL_SCHEDULE *=/ {print $2}' $HACK_INI)
STORAGE_CIFS=$(awk -F "=" '/STORAGE_CIFS *=/ { gsub(/^\/*/, "", $2);print $2}' $HACK_INI)
STORAGE_CIFS_PATH=$(awk -F "=" '/STORAGE_CIFS_PATH *=/ { gsub(/^\/*/, "", $2);print $2}' $HACK_INI)
STORAGE_SDCARD=$(awk -F "=" '/STORAGE_SDCARD *=/ {print $2}' $HACK_INI)
WEBHOOK=$(awk -F "=" '/WEBHOOK *=/ {print $2}' $HACK_INI)
WEBHOOK_URL=$(awk -F "=" '/WEBHOOK_URL *=/ {print $2}' $HACK_INI)
WEBHOOK_RECORD_EVENT=$(awk -F "=" '/WEBHOOK_RECORD_EVENT *=/ {print $2}' $HACK_INI)
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

if [ "$FMT" != "" ]; then
  TMPFILE="/tmp/mv_`cat /proc/sys/kernel/random/uuid`"
  mv $1 $TMPFILE
  (
    if [ "$STORAGE_CIFS" = "on" -o "$STORAGE_CIFS" = "record" ] && /tmp/system/bin/mount_cifs ; then
      TIME=`echo $2 | sed -e 's|^/media/mmc/record/||' -e 's|/||g' -e 's|.mp4$||'`
      OUTFILE=`TZ=JST-9 date -d $TIME +"/mnt/$HOSTNAME/record/$STORAGE_CIFS_PATH.mp4"`
      DIR_PATH=${OUTFILE%/*}
      mkdir -p $DIR_PATH
      cp $TMPFILE $OUTFILE
    fi

    if [ "$STORAGE_SDCARD" = "on" -o "$STORAGE_SDCARD" = "record" ]; then
      /bin/busybox mv $TMPFILE $2 || /bin/busybox rm $TMPFILE
    else
      /bin/busybox rm $TMPFILE
      LD_LIBRARY_PATH=/tmp/system/lib:/usr/lib /tmp/system/lib/ld.so.1 /tmp/system/bin/find_libc /media/mmc/record -depth -type d -empty -delete
    fi
    if [ "$WEBHOOK" = "on" ] && [ "$WEBHOOK_URL" != "" ]; then
      if [ "$WEBHOOK_RECORD_EVENT" = "on" ]; then
        LD_LIBRARY_PATH=/tmp/system/lib:/usr/lib /tmp/system/lib/ld.so.1 /tmp/system/bin/curl -X POST -m 3 -H "Content-Type: application/json" -d "{\"type\":\"recordEvent\", \"device\":\"${HOSTNAME}\"}" $WEBHOOK_URL > /dev/null 2>&1
      fi
    fi
  ) &
else
  /bin/busybox rm $1
fi
exit 0
