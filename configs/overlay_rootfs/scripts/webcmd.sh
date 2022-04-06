#!/bin/sh

mkfifo /var/run/webcmd
chmod 666 /var/run/webcmd
mkfifo /var/run/webres
chmod 666 /var/run/webres
tail -F /var/run/webcmd | while read line
do
  echo "[webcmd] $line"
  cmd=${line%% *}
  params=${line#* }
  if [ "$cmd" = "reboot" ]; then
    echo "$cmd $params OK" >> /var/run/webres
    killall -SIGUSR2 iCamera_app
    sync
    sync
    sync
    reboot
    cmd=""
  fi
  if [ "$cmd" = "scheduleReboot" ]; then
    /scripts/set_crontab.sh
  fi
  if [ "$cmd" = "setwebhook" ]; then
    kill -9 `pidof webhook.sh`
    /scripts/webhook.sh &
    echo "$cmd $params OK" >> /var/run/webres
    cmd=""
  fi
  if [ "$cmd" = "hostname" ] && [ "$params" != "" ]; then
    echo ${params%%.*} > /media/mmc/hostname
    hostname ${params%%.*}
    if [ "`pidof avahi-daemon`" != "" ]; then
      /usr/sbin/avahi-daemon -k
      /usr/sbin/avahi-daemon -D
    fi
    if [ "`pidof nmbd`" != "" ]; then
      killall -9 nmbd
      nmbd -D
    fi
    echo "$cmd $params OK" >> /var/run/webres
    cmd=""
  fi
  if [ "$cmd" = "rtspserver" ] && [ "$params" != "" ]; then
    /scripts/rtspserver.sh $params
    echo "$cmd $params OK" >> /var/run/webres
    cmd=""
  fi
  if [ "$cmd" = "cruise" ]; then
    kill -9 `pidof cruise.sh`
    /scripts/cruise.sh &
    echo "$cmd $params OK" >> /var/run/webres
    cmd=""
  fi
  if [ "$cmd" = "lighttpd" ]; then
    echo "$cmd OK" >> /var/run/webres
    sleep 3
    /scripts/lighttpd.sh restart
    cmd=""
  fi
  if [ "$cmd" = "samba" ] && [ "$params" != "" ]; then
    /scripts/samba.sh $params
    echo "$cmd $params OK" >> /var/run/webres
    cmd=""
  fi
  if [ "$cmd" = "sderase" ]; then
    busybox rm -rf /media/mmc/record
    busybox rm -rf /media/mmc/alarm_record
    busybox rm -rf /media/mmc/time_lapse
    echo "$cmd $params OK" >> /var/run/webres
    cmd=""
  fi
  if [ "$cmd" = "update" ]; then
    HACK_INI=/tmp/hack.ini
    CUSTOM_ZIP=$(awk -F "=" '/CUSTOM_ZIP *=/ {print $2}' $HACK_INI)
    ZIP_URL=$(awk -F "=" '/CUSTOM_ZIP_URL *=/ {print $2}' $HACK_INI)
    if [ "$CUSTOM_ZIP" = "off" ] || [ "$ZIP_URL" = "" ]; then
      ZIP_URL="https://github.com/mnakada/atomcam_tools/releases/latest/download/atomcam_tools.zip"
    fi
    mkdir -p /media/mmc/update
    (cd /media/mmc/update; curl -H 'Cache-Control: no-cache, no-store' -sL -o atomcam_tools.zip $ZIP_URL)
    echo "$cmd $params OK" >> /var/run/webres
    sleep 1
    killall -SIGUSR2 iCamera_app
    sync
    sync
    sync
    reboot
    cmd=""
  fi
  if [ "$cmd" = "posrec" ]; then
    pos=`/scripts/cmd move`;
    awk '
    /slide_x/ {
      pan=POS;
      gsub(/ .*$/, "", pan);
      printf("slide_x=%d\n", int(pan * 100 + 0.5));
      next;
    }
    /slide_y/ {
      tilt=POS;
      gsub(/^.* /, "", tilt);
      printf("slide_y=%d\n", int(tilt * 100 + 0.5));
      next;
    }
    {
      print;
    }
    ' POS="$pos" /atom/configs/.user_config > /atom/configs/.user_config_new
    mv -f /atom/configs/.user_config_new /atom/configs/.user_config
  fi
  if [ "$cmd" = "moveinit" ]; then
    /scripts/motor_init
  fi
  if [ "$cmd" != "" ]; then
    echo "$cmd $param : syntax error" >> /var/run/webres
  fi
done
