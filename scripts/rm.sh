#!/bin/sh
set -x


if [ -e $1 ]; then
#  echo "$1 is file parameter"
        FILE=$1 
        OPT=$2
fi
 
if [ -e $2 ]; then
#  echo "$2 is file parameter"
        FILE=$2
        OPT=$1
fi

if [[ "$FILE" = "/tmp/alarm.jpg" || "$FILE" = "/tmp/alarm_record.mp4" ]]; then
	HACK_INI=/tmp/mmc/hack.ini
	ALARMREC=$(awk -F "=" '/ALARMREC/ {print $2}' $HACK_INI)
	FTPCLIENT=$(awk -F "=" '/FTPCLIENT/ {print $2}' $HACK_INI)
	FTPADDR=$(awk -F "=" '/FTPADDR/ {print $2}' $HACK_INI)
	FTPUSER=$(awk -F "=" '/FTPUSER/ {print $2}' $HACK_INI)
	FTPPASS=$(awk -F "=" '/FTPPASS/ {print $2}' $HACK_INI)
	FTPFOLDER=$(awk -F "=" '/FTPFOLDER/ {print $2}' $HACK_INI)
	FTPTRANSALARM=$(awk -F "=" '/FTPTRANSALARM/ {print $2}' $HACK_INI)
	FMT=`TZ=JST-9 date +%Y%m%d_%H%M%S"`
	echo $FMT > /dev/console

#		str=`echo ${FILE} | awk -F "/" '{ print $NF }'`
#		echo "basename:${str}"

	OUTPATH="/media/mmc/alarm_record/"$FMT".mp4"
	echo "outpath:$OUTPATH" > /dev/console

	if [ $ALARMREC = "on" ]; then
		if [ "$FILE" = "/tmp/alarm_record.mp4" ]; then
			mkdir /media/mmc/alarm_record/
			cp $FILE $OUTPATH
		fi
	fi


if [ $FTPCLIENT = "on" ]; then
	if [ $FTPTRANSALARM = "on" ]; then
		if [ "$FILE" = "/tmp/alarm_record.mp4" ]; then
	        
		  /tmp/newroot/usr/bin/lftp -e "mkdir -p $FTPFOLDER/alarm_record; put -O $FTPFOLDER/alarm_record $FILE -o $FMT.mp4 ; quit" ftp://$FTPADDR -u $FTPUSER,$FTPPASS

		fi

    fi
fi




#	if [ -e /media/mmc/scripts/alarmhook.sh ]; then
#		if [ "$FILE" = "/tmp/alarm_record.mp4" ]; then
#			source /media/mmc/scripts/alarmhook.sh $OUTPATH
#		fi
#	fi

fi

busybox rm $1 $2
