#!/bin/sh

set -x
export PATH='/media/mmc/scripts:/system/bin:/bin:/sbin:/usr/bin:/usr/sbin'

ProcessName=telnetd
while true
do

# telnet killed by iCamera_app

	count=`ps | grep $ProcessName | grep -v grep | wc -l`
	if [ $count = 0 ]; then
	  echo "$ProcessName is dead."
	  telnetd 
	else
	  echo "$ProcessName is alive." > /dev/null
	fi

sleep 10

done
