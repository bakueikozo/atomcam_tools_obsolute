#!/bin/sh

export PATH=/bin:/sbin:/usr/bin:/usr/sbin
HACK_INI=/media/mmc/hack.ini

while true
do
#	echo 'check reboot time'
	REBOOTEACH=$(awk -F "=" '/REBOOTEACH/ {print $2}' $HACK_INI)
	if [ -z "$REBOOTEACH" ]; then
		REBOOTEACH=0
	fi

  if [ $REBOOTEACH -ne 0 ]; then
    UPTIME=`cat /proc/uptime | cut -f 1 -d ' ' | awk '{printf("%d\n",$1)}'`
    REBOOTSEC=`expr $REBOOTEACH '*' 3600`
    if [ $REBOOTSEC -lt $UPTIME ]; then
      echo 'REBOOOOOOT!!!!!'
      reboot
    fi
  fi

# ftp test
	if [ -f /tmp/ftptest ]; then
		killall -9 lftp
		FTPCLIENT=$(awk -F "=" '/FTPCLIENT/ {print $2}' $HACK_INI)
		FTPADDR=$(awk -F "=" '/FTPADDR/ {print $2}' $HACK_INI)
		FTPUSER=$(awk -F "=" '/FTPUSER/ {print $2}' $HACK_INI)
		FTPPASS=$(awk -F "=" '/FTPPASS/ {print $2}' $HACK_INI)
		FTPFOLDER=$(awk -F "=" '/FTPFOLDER/ {print $2}' $HACK_INI)
		FTPTRANSNORMAL=$(awk -F "=" '/FTPTRANSNORMAL/ {print $2}' $HACK_INI)
		rm /tmp/log/ftp.log
		rm /tmp/log/ftperr.log
    rm /tmp/log/ftptest.log
		dd if=/dev/urandom of=/tmp/test.bin bs=1024 count=1
		TZ=JST-9 lftp -e "set xfer:log-file /tmp/log/ftp.log; set net:timeout 60; set net:max-retries 3 ;set net:reconnect-interval-base 10; open -u $FTPUSER,$FTPPASS $FTPADDR ; mkdir -p $FTPFOLDER/ftptest ; put -O $FTPFOLDER/ftptest /tmp/test.bin -o test.bin; rm $FTPFOLDER/ftptest/test.bin ; rmdir $FTPFOLDER/ftptest; quit" 2>/tmp/log/ftperr.log
		cat /tmp/log/ftperr.log > /tmp/log/ftptest.log
		cat /tmp/log/ftp.log >> /tmp/log/ftptest.log
		rm -rf /tmp/log/ftp.log
    rm -rf /tmp/log/ftperr.log
    rm -rf /tmp/ftptest
	fi

# reboot
	if [ -f /tmp/cmd_reboot ]; then
		sync;sync;sync;
		reboot
	fi

	sleep 1
done

