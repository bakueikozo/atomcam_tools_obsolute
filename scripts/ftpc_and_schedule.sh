#!/bin/sh

set -x
echo "------ftpclient.sh---------"
export PATH='/bin:/sbin:/usr/bin:/usr/sbin'
HACK_INI=/media/mmc/hack.ini
while true
do
	NOW=`TZ=JST-9 date +"%Y%m%d%H%M%S"`
	FLAG=${NOW:0:1}
	if [ $FLAG == "2" ]; then
		break;
	fi
	sleep 30
done

while true
do
	SCHEDULE=$(awk -F "=" '/SCHEDULE/ {print $2}' $HACK_INI)
	if [ "$SCHEDULE" = "scheduled" ]; then
		MON=$(awk -F "=" '/MON/ {print $2}' $HACK_INI)
		TUE=$(awk -F "=" '/TUE/ {print $2}' $HACK_INI)
		WED=$(awk -F "=" '/WED/ {print $2}' $HACK_INI)
		THU=$(awk -F "=" '/THU/ {print $2}' $HACK_INI)
		FRI=$(awk -F "=" '/FRI/ {print $2}' $HACK_INI)
		SAT=$(awk -F "=" '/SAT/ {print $2}' $HACK_INI)
		SUN=$(awk -F "=" '/SUN/ {print $2}' $HACK_INI)

		DWW=0
		NOW=`TZ=JST-9 date +"%Y%m%d%H%M%S"`
		YEAR=${NOW:0:4}
		MONTH=${NOW:4:2}
		DAY=${NOW:6:2}
		HOUR=${NOW:8:2}

		for d in "SUN" "MON" "TUE" "WED" "THU" "FRI" "SAT"
		do
	  	DW=`TZ=JST-9 date "+%w"`
			if [ "$DW" = "$DWW" ] ; then
				echo "today is $d";
				echo -n "schedule is "
				SCH=`eval echo "'$'$d"`
				echo $SCH
			fi
		  DWW=`expr $DWW + 1`
		done

		CUR=${SCH:$HOUR:1}
		echo "current schedule is $CUR"
		if [ "$CUR" = "0" ] ; then
			echo "remove /media/mmc/record/$YEAR$MONTH$DAY/$HOUR/*.mp4"
			rm /media/mmc/record/$YEAR$MONTH$DAY/$HOUR/*.mp4
			touch /media/mmc/record/$YEAR$MONTH$DAY/$HOUR/.removed_by_scheduled
		else
			echo "all record."
		fi
	fi

	FTPCLIENT=$(awk -F "=" '/FTPCLIENT/ {print $2}' $HACK_INI)
	FTPADDR=$(awk -F "=" '/FTPADDR/ {print $2}' $HACK_INI)
	FTPUSER=$(awk -F "=" '/FTPUSER/ {print $2}' $HACK_INI)
	FTPPASS=$(awk -F "=" '/FTPPASS/ {print $2}' $HACK_INI)
	FTPFOLDER=$(awk -F "=" '/FTPFOLDER/ {print $2}' $HACK_INI)
	FTPTRANSNORMAL=$(awk -F "=" '/FTPTRANSNORMAL/ {print $2}' $HACK_INI)

	if [ -z $FTPFOLDER ]; then
		FTPFOLDER="."
	fi

	if [ $FTPCLIENT = "on" ]; then
		if [ $FTPTRANSNORMAL = "on" ]; then
			cd /media/mmc/record
      echo "check mmc mp4"
      FILE=`find . -name "*.mp4" | sort | tail -n 1`
			if [ $FILE = $LASTFILE ]; then
			  echo "not changed"
			else
        echo -n "file updated [$FILE]"
		        
				TARGET=${FILE:2//\// };
				TARGET=`echo $TARGET | sed s:/::g`
				YEAR=${TARGET:0:4}
				MONTH=${TARGET:4:2}
				DAY=${TARGET:6:2}
				TIME=${TARGET:8:4}

				REMOTEFULLPATH="record/$YEAR/$MONTH/$DAY/$YEAR$MONTH$DAY""_""$TIME.mp4"
				REMOTEPATH=`dirname $REMOTEFULLPATH`
				REMOTEFILE=`basename $REMOTEFULLPATH`
				lftp -e "set net:timeout 60; set net:max-retries 3 ;set net:reconnect-interval-base 10; mkdir -p $FTPFOLDER/$REMOTEPATH; put -O $FTPFOLDER/$REMOTEPATH $FILE  -o $REMOTEFILE ; quit" ftp://$FTPADDR -u $FTPUSER,$FTPPASS
			fi
		    LASTFILE=$FILE
	    fi
	fi
	sleep 1
done



