#!/bin/sh

echo "Cache-Control: no-cache"
echo "Content-Type: plain/text"
echo ""

if [ "$REQUEST_METHOD" = "POST" ]; then
  awk '
  BEGIN {
    RS="[{},]";
  }
  /^$/ { next; }
  /appver/ { next; }
  /PRODUCT_MODEL/ { next; }
  /HOSTNAME/ { next; }
  {
    gsub(/\"[ \t]*:[ \t]*\"/, "=");
    gsub(/\"/, "");
    print;
  }
  ' > /media/mmc/hack.ini

  REBOOT_SCHEDULE = `awk '/REBOOT_SCHEDULE *=/ { gsub(/^REBOOT_SCHEDULE *=/, ''); print; }' /media/mmc/hack.ini`
  awk '
  /\/scripts\/reboot\.sh/ {
    next;
  }
  {
    print;
  }
  END {
    if(REBOOT_SCHEDULE != "") print REBOOT_SCHEDULE " /scripts/reboot.sh";
  }
  ' -v REBOOT_SCHEDULE="$REBOOT_SCHEDULE" /var/spool/cron/crontabs/root > /var/spool/cron/crontabs/root.new
  busybox mv /var/spool/cron/crontabs/root.new /var/spool/cron/crontabs/root

  awk '
  BEGIN {
    FS=";";
  }
  /RECORDING_LOCAL_SCHEDULE_LIST *=/ {
    gsub(/^RECORDING_LOCAL_SCHEDULE_LIST *=/, '');
    for(i = 1; i <= NF; i++) {
      print $i;
    }
  }
  ' /media/mmc/hack.ini > /media/mmc/local_schedule

  exit 0
fi

awk '/appver/ { print }' /atom/configs/app.ver
awk '/PRODUCT_MODEL/ { print }' /atom/configs/.product_config
echo "HOSTNAME=`hostname`"
echo "KERNELVER=`uname -a`"
cat /media/mmc/hack.ini
