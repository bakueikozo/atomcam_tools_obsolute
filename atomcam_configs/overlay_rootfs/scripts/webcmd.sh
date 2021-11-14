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
    sync
    sync
    sync
    reboot
    cmd=""
  fi
  if [ "$cmd" = "scheduleReboot" ]; then
    cat << EOF | crontab -
*/15 * * * * /usr/sbin/logrotate /etc/logrotate.conf
${params} /scripts/reboot.sh
EOF
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
    if [ "$params" = "on" ]; then
      while [ "`pidof v4l2rtspserver`" = "" ]
      do
        sleep 1
      done
    fi
    echo "$cmd $params OK" >> /var/run/webres
    cmd=""
  fi
  if [ "$cmd" = "samba" ] && [ "$params" != "" ]; then
    if [ "$params" = "on" ]; then
      pidof smbd || smbd -D;
      pidof nmbd || nmbd -D;
      while [ "`pidof smbd`" = "" ]
      do
        sleep 1
      done
      while [ "`pidof nmbd`" = "" ]
      do
        sleep 1
      done
    fi
    if [ "$params" = "off" ]; then
      kill -9 `pidof smbd`
      kill -9 `pidof nmbd`
    fi
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

  if [ "$cmd" != "" ]; then
    echo "$cmd $param : syntax error" >> /var/run/webres
  fi
done
