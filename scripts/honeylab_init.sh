#!/bin/sh

#set -x

echo '\n------------honeylab_init------------'
export PATH='/media/mmc/scripts:/system/bin:/bin:/sbin:/usr/bin:/usr/sbin'
HACK_INI=/tmp/mmc/hack.ini

echo "called from app_init.sh"
echo "overmount busybox and passwords"

cp /tmp/Test/busybox /tmp
cp /tmp/Test/passwd /tmp
cp /tmp/Test/shadow /tmp



cp -r /bin /tmp/bin
rm -r /tmp/bin/rm

mount -o bind /tmp/busybox /bin/busybox
mount -o bind /tmp/bin /bin
cp /tmp/mmc/scripts/rm.sh /bin/rm
chmod 755 /bin/rm

echo "insmod ext2 driver"
insmod /tmp/mmc/modules/crc16.ko
insmod /tmp/mmc/modules/mbcache.ko
insmod /tmp/mmc/modules/jbd2.ko
insmod /tmp/mmc/modules/ext4.ko

echo "mount ext2 new rootfs"
mkdir /tmp/newroot
mount -t ext2 /tmp/mmc/rootfs_hack.ext2 /tmp/newroot

mount -o bind /tmp/passwd /etc/passwd
mount -o bind /tmp/shadow /etc/shadow

SWAPSIZE=$(awk -F "=" '/SWAPSIZE/ {print $2}' $HACK_INI)

if [ -z "$SWAPSIZE" ]; then
	SWAPSIZE=0
fi

if [ $SWAPSIZE = 0 ]; then
	echo " no swap"
	/tmp/busybox rm /tmp/mmc/swap
else
	dd if=/dev/zero of=/tmp/mmc/swap bs=1M count=$SWAPSIZE
	mkswap /tmp/mmc/swap
	swapon /tmp/mmc/swap
fi


ln -s   /usr/boa/boa /tmp/boa2
cp /usr/boa/boa.conf /tmp/boa.conf
sed -e "s/Port 80/Port 8080/g" /tmp/boa.conf > /tmp/boa2.conf
mount -o bind /tmp/boa2.conf /usr/boa/boa.conf
/tmp/boa2
umount /usr/boa/boa.conf


cp -r /usr /tmp/newroot/mnt
mkdir /tmp/newroot/mnt/usr/lib
mount /tmp/newroot/usr/lib /tmp/newroot/mnt/usr/lib
mount -o rbind /tmp/newroot/mnt/usr /usr


mkdir /tmp/www
mkdir /tmp/www/cgi-bin
chmod 755 /tmp/www/cgi-bin
cp /tmp/mmc/scripts/honeylab.cgi /tmp/www/cgi-bin
chmod 755 /tmp/www/cgi-bin/honeylab.cgi


echo "run /tmp/mmc/scripts/pre.sh"
source /tmp/mmc/scripts/pre.sh

/system/bin/hl_client &

RTSPSERVER=$(awk -F "=" '/RTSPSERVER/ {print $2}' $HACK_INI)
if [ $RTSPSERVER = "on" ]; then

	insmod /tmp/mmc/modules/v4l2loopback.ko video_nr=1
	LD_PRELOAD=/tmp/mmc/modules/libcallback.so /system/bin/iCamera_app &
else
	/system/bin/iCamera_app &

fi

/system/bin/dongle_app &


FTPSERVER=$(awk -F "=" '/FTPSERVER/ {print $2}' $HACK_INI)
if [ $FTPSERVER = "on" ]; then
	/tmp/busybox tcpsvd -vE 0.0.0.0 21 /tmp/busybox ftpd / &
fi

if [ $RTSPSERVER = "on" ]; then
# for enable rtsp server , remove coment merker
 sleep 15
 insmod /tmp/mmc/modules/v4l2loopback.ko video_nr=1
 LD_LIBRARY_PATH=/tmp/newroot/lib:/tmp/newroot/usr/lib:/lib:/usr/lib /tmp/newroot/lib/ld.so.1 /tmp/newroot/usr/bin/v4l2rtspserver  /dev/video1 &
fi

echo "run /tmp/mmc/post.sh"
source /tmp/mmc/scripts/post.sh

/tmp/mmc/scripts/ftpc_and_schedule.sh &
/tmp/mmc/scripts/telnet.sh &


while true
do

#	echo 'retain in honeylab_init'

#	echo 'check reboot time'
	REBOOTEACH=$(awk -F "=" '/REBOOTEACH/ {print $2}' $HACK_INI)
	if [ -z "$REBOOTEACH" ]; then
		REBOOTEACH=0
	fi

	if [ $REBOOTEACH -eq 0 ]; then
		echo 'REBOOTEACH is disabled'
	else
		if [ $REBOOTEACH -ne 0 ]; then
			UPTIME=`cat /proc/uptime | cut -f 1 -d ' ' | awk '{printf("%d\n",$1)}`
			echo $UPTIME
			REBOOTSEC=`/tmp/busybox expr $REBOOTEACH '*' 3600`
			echo $REBOOTSEC
			if [ $REBOOTSEC -lt $UPTIME ]; then
				echo 'REBOOOOOOT!!!!!'
				reboot
			else
				echo 'time is not reached'
			fi
			
		
		fi
	fi
	ifconfig wlan0 | awk '/inet / {print $2}' | awk -F: '{print $2}' > /tmp/ipaddr

	if [ -f /tmp/ftptest ]; then
	
		FTPCLIENT=$(awk -F "=" '/FTPCLIENT/ {print $2}' $HACK_INI)
		FTPADDR=$(awk -F "=" '/FTPADDR/ {print $2}' $HACK_INI)
		FTPUSER=$(awk -F "=" '/FTPUSER/ {print $2}' $HACK_INI)
		FTPPASS=$(awk -F "=" '/FTPPASS/ {print $2}' $HACK_INI)
		FTPFOLDER=$(awk -F "=" '/FTPFOLDER/ {print $2}' $HACK_INI)
		FTPTRANSNORMAL=$(awk -F "=" '/FTPTRANSNORMAL/ {print $2}' $HACK_INI)
		/tmp/busybox rm /tmp/ftp.log
		/tmp/busybox rm /tmp/ftperr.log
		/tmp/busybox rm /tmp/ftptest.log
		dd if=/dev/urandom of=/tmp/test.bin bs=1024 count=1
		TZ=JST-9 LD_LIBRARY_PATH=/tmp/newroot/lib:/tmp/newroot/usr/lib:/lib:/usr/lib /tmp/newroot/lib/ld.so.1 /tmp/newroot/usr/bin/lftp -e "set xfer:log-file /tmp/ftp.log; set net:timeout 60; set net:max-retries 3 ;set net:reconnect-interval-base 10; open -u $FTPUSER,$FTPPASS $FTPADDR ; mkdir -p $FTPFOLDER/ftptest ; put -O $FTPFOLDER/ftptest /tmp/test.bin -o test.bin; rm $FTPFOLDER/ftptest/test.bin ; rmdir $FTPFOLDER/ftptest; quit" 2>/tmp/ftperr.log
		cat /tmp/ftperr.log > /tmp/ftptest.log
		cat /tmp/ftp.log >> /tmp/ftptest.log
		busybox rm -rf /tmp/ftp.log
		busybox rm -rf /tmp/ftperr.log
		busybox rm -rf /tmp/ftptest
	fi 
	if [ -f /tmp/cmd_reboot ]; then
		sleep 10
		sync;sync;sync;
		reboot
	fi
	sleep 2

done

