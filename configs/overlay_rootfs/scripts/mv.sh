#!/bin/sh
#
# for continuous recording file
#

if [ "${1%/*.mp4}" != "/tmp" ] || [ "${2##/media/mmc/record/*.mp4}" != "" ] ; then
  /bin/busybox ${0##*/} $*
  exit
fi

HACK_INI=/media/mmc/hack.ini
RECORDING_LOCAL_SCHEDULE=$(awk -F "=" '/RECORDING_LOCAL_SCHEDULE *=/ {print $2}' $HACK_INI)
RECORDING_PATH=$(awk -F "=" '/RECORDING_PATH *=/ { gsub(/^\/*/, "", $2);print $2}' $HACK_INI)
STORAGE_SDCARD=$(awk -F "=" '/STORAGE_SDCARD *=/ {print $2}' $HACK_INI)
STORAGE_CIFS=$(awk -F "=" '/STORAGE_CIFS *=/ {print $2}' $HACK_INI)
STORAGE_CIFSSERVER=$(awk -F "=" '/STORAGE_CIFSSERVER *=/ {gsub(/\/$/, "", $2); print $2}' $HACK_INI)
STORAGE_CIFSUSER=$(awk -F "=" '/STORAGE_CIFSUSER *=/ {print $2}' $HACK_INI)
STORAGE_CIFSPASSWD=$(awk -F "=" '/STORAGE_CIFSPASSWD *=/ {print $2}' $HACK_INI)
WEBHOOK=$(awk -F "=" '/WEBHOOK *=/ {print $2}' $HACK_INI)
WEBHOOK_URL=$(awk -F "=" '/WEBHOOK_URL *=/ {print $2}' $HACK_INI)
WEBHOOK_RECORD_EVENT=$(awk -F "=" '/WEBHOOK_RECORD_EVENT *=/ {print $2}' $HACK_INI)
HOSTNAME=`hostname`

USER_CONFIG=/configs/.user_config
SCHEDULE_CONFIG=/configs/.multiplealarm_config
ALARMDATE=$(awk -F "=" '/alarmDate *=/ {print $2}' $USER_CONFIG)
RECORDTYPE=$(awk -F "=" '/recordType *=/ {print $2}' $USER_CONFIG)
if [ "$RECORDTYPE" = "1" ] && [ "$RECORDING_LOCAL_SCHEDULE" = "on" ]; then
  ALARMDATE= "0"
  SCHEDULE_CONFIG=/media/mmc/local_schedule
fi
if [ "$ALARMDATE" = "0" ] && [ -f $SCHEDULE_CONFIG ]; then
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
  ' ALARMDATE=$ALARMDATE $SCHEDULE_CONFIG`
else
  FMT=`TZ=JST-9 date +"%Y%m%d_%H%M%S"`
fi

if [ "$FMT" != "" ]; then
  if [ "$STORAGE_CIFS" = "on" ] && [ "$STORAGE_CIFSSERVER" != "" ]; then
    if ! mount | grep "$STORAGE_CIFSSERVER" > /dev/null ; then
      LD_LIBRARY_PATH=/tmp/system/lib:/usr/lib:/usr/lib/samba /tmp/system/lib/ld.so.1 /tmp/system/bin/busybox mount -t cifs -ousername=$STORAGE_CIFSUSER,password=$STORAGE_CIFSPASSWD,vers=3.0 $STORAGE_CIFSSERVER /mnt
    fi
    if [ $? = 0 ]; then
      TIME=`echo $2 | sed -e 's|^/media/mmc/record/||' -e 's|/||g' -e 's|.mp4$||'`
      OUTFILE=`TZ=JST-9 date -d $TIME +"/mnt/$HOSTNAME/record/$RECORDING_PATH.mp4"`
      DIR_PATH=${OUTFILE%/*}
      mkdir -p $DIR_PATH
      cp $1 $OUTFILE
    fi
  fi

  if [ "$STORAGE_SDCARD" = "on" ]; then
    /bin/busybox mv $*
  else
    /bin/busybox rm $1
  fi
  if [ "$WEBHOOK" = "on" ] && [ "$WEBHOOK_URL" != "" ]; then
    if [ "$WEBHOOK_RECORD_EVENT" = "on" ]; then
       LD_LIBRARY_PATH=/tmp/system/lib:/usr/lib /tmp/system/lib/ld.so.1 /tmp/system/bin/curl -X POST -H "Content-Type: application/json" -d "{\"type\":\"recordEvent\", \"device\":\"${HOSTNAME}\"}" $WEBHOOK_URL
    fi
  fi
else
  /bin/busybox rm $1
fi
