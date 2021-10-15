#!/bin/sh

#set -x

export PATH=/bin:/sbin:/usr/bin:/usr/sbin
HACK_INI=/media/mmc/hack.ini
RTSPSERVER=$(awk -F "=" '/RTSPSERVER/ {print $2}' $HACK_INI)

# atomcam system
if [ "$0" != "atom_mounted"] ; then
  mount -a
  mkdir /atom
  mount -t squashfs /dev/mtdblock2 /atom
  mount -t squashfs /dev/mtdblock3 /atom/system
  mount -t jffs2 /dev/mtdblock6 /atom/configs
fi
mount -t proc proc /atom/proc
mount -o rbind /dev /atom/dev
mount -t sysfs sysfs /atom/sys
mount -o bind /tmp /atom/tmp
mount -o rbind /media /atom/media
mkdir /system
mount -o bind /atom/system /system

# sawap
SWAPSIZE=$(awk -F "=" '/SWAPSIZE/ {print $2}' $HACK_INI)
[ -z "$SWAPSIZE" ] && SWAPSIZE=0
if [ $SWAPSIZE = 0 ]; then
	echo " no swap"
	rm /media/mmc/swap
else
	dd if=/dev/zero of=/media/mmc/swap bs=1M count=$SWAPSIZE
	mkswap /media/mmc/swap
	swapon /media/mmc/swap
fi

# /root
mkdir -p /root/etc_default
ln -sf /atom/system/etc/wpa_supplicant.conf /root/etc_default/wpa_supplicant.conf
mkdir -p /root/.ssh
chmod 700 /root/.ssh
[ -f /media/mmc/authorized_keys ] && cp /media/mmc/authorized_keys /root/.ssh
[ -f /root/.profile ] || cat << EOF > /root/.profile
if [ "$PS1" ]; then
	if [ "`id -u`" -eq 0 ]; then
		export PS1='[\u@\h:\W]# '
	else
		export PS1='[\u@\h:\W]$ '
	fi
fi
export PATH=/bin:/sbin:/usr/bin:/usr/sbin
umask 022
EOF

# atomcam patch for assis external commands
cp /atom/lib/ld-uClibc* /lib
mkdir /tmp/system_bin
cp /atom/system/bin/* /tmp/system_bin
cp /media/mmc/scripts/rm.sh /tmp/system_bin/rm
chmod 755 /tmp/system_bin/rm
for i in `ls /tmp/system_bin`
do
  if file /tmp/system_bin/$i | grep 'uClibc' > /dev/null ; then
    mv /tmp/system_bin/$i /tmp/system_bin/${i}_org
    cp /media/mmc/scripts/atom_patch.sh /tmp/system_bin/$i
    chmod 755 /tmp/system_bin/$i
  fi
done
mount -o bind /tmp/system_bin /system/bin

# atomcam iCamera_app
[ $RTSPSERVER = "on" ] && insmod /media/mmc/modules/v4l2loopback.ko video_nr=1
PATH=/system/bin:/bin:/sbin:/usr/bin:/usr/sbin chroot /atom /media/mmc/scripts/atom_init.sh $RTSPSERVER

# /etc/hostname
[ ! -f /etc/hostname ] && echo "atomcam" > /etc/hostname
[ -f /media/mmc/hostname ] && cp /media/mmc/hostname /etc/hostname
hostname -F /etc/hostname

# ssh
mkdir -p /var/empty
/usr/bin/ssh-keygen -A
/usr/sbin/sshd

#avahi
/usr/sbin/avahi-daemon -D

#boa
mkdir /tmp/www
mkdir /tmp/www/cgi-bin
chmod 755 /tmp/www/cgi-bin
cp /media/mmc/scripts/honeylab.cgi /tmp/www/cgi-bin
chmod 755 /tmp/www/cgi-bin/honeylab.cgi
cp /media/mmc/scripts/get_jpeg.cgi /tmp/www/cgi-bin
chmod 755 /tmp/www/cgi-bin/get_jpeg.cgi
cp /media/mmc/scripts/still_image.html /tmp/www
/usr/sbin/boa -f /media/mmc/scripts/boa.conf

# ftp server
FTPSERVER=$(awk -F "=" '/FTPSERVER/ {print $2}' $HACK_INI)
[ $FTPSERVER = "on" ] && tcpsvd -vE 0.0.0.0 21 ftpd / &

# rtsp server
[ $RTSPSERVER = "on" ] && (sleep 15; /usr/bin/v4l2rtspserver /dev/video1) &

# ftp client and scheduler
/media/mmc/scripts/ftpc_and_schedule.sh &

# telnetd
TELNETD=$(awk -F "=" '/TELNETD/ {print $2}' $HACK_INI)
[ $TELNETD = "on" ] && /media/mmc/scripts/telnet.sh &

while true
do
#	echo 'check reboot time'
	REBOOTEACH=$(awk -F "=" '/REBOOTEACH/ {print $2}' $HACK_INI)
	if [ -z "$REBOOTEACH" ]; then
		REBOOTEACH=0
	fi

	if [ $REBOOTEACH -eq 0 ]; then
		echo 'REBOOTEACH is disabled'
	else
		if [ $REBOOTEACH -ne 0 ]; then
			UPTIME=`cat /proc/uptime | cut -f 1 -d ' ' | awk '{printf("%d\n",$1)}'`
			echo $UPTIME
			REBOOTSEC=`expr $REBOOTEACH '*' 3600`
			echo $REBOOTSEC
			if [ $REBOOTSEC -lt $UPTIME ]; then
				echo 'REBOOOOOOT!!!!!'
				reboot
			else
				echo 'time is not reached'
			fi
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

