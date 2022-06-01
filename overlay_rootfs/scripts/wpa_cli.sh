#!/bin/sh
echo 'wpa_state=COMPLETED'
echo ip_addresss=`/bin/busybox ifconfig wlan0 | awk '/inet / {gsub(/^.*:/, "", $2);print $2}'`
