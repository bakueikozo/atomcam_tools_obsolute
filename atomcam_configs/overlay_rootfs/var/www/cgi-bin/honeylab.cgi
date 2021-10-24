#!/bin/sh
echo Content-type: text/html

echo 
echo

HACK_INI=/media/mmc/hack.ini
SAVE_INI=/media/mmc/hack.ini

FTPSERVER="off"
FTPCLIENT="off"
TELNETD="off"
RTSPSERVER="off"
ALARMREC="off"
FTPTRANSNORMAL="off"
FTPTRANSALARM="off"

load_ini(){
	
	ALARMREC=$(awk -F "=" '/ALARMREC/ {print $2}' $HACK_INI)
	IFTTTURL=$(awk -F "=" '/IFTTT_URL/ {print $2}' $HACK_INI)
	FTPSERVER=$(awk -F "=" '/FTPSERVER/ {print $2}' $HACK_INI)
	FTPCLIENT=$(awk -F "=" '/FTPCLIENT/ {print $2}' $HACK_INI)

	FTPADDR=$(awk -F "=" '/FTPADDR/ {print $2}' $HACK_INI)
	FTPUSER=$(awk -F "=" '/FTPUSER/ {print $2}' $HACK_INI)
	FTPPASS=$(awk -F "=" '/FTPPASS/ {print $2}' $HACK_INI)
	FTPFOLDER=$(awk -F "=" '/FTPFOLDER/ {print $2}' $HACK_INI)
	FTPRECORD=$(awk -F "=" '/FTPRECORD/ {print $2}' $HACK_INI)

	FTPTRANSALARM=$(awk -F "=" '/FTPTRANSALARM/ {print $2}' $HACK_INI)
	FTPTRANSNORMAL=$(awk -F "=" '/FTPTRANSNORMAL/ {print $2}' $HACK_INI)

	SWAPSIZE=$(awk -F "=" '/SWAPSIZE/ {print $2}' $HACK_INI)
	if [ -z "$SWAPSIZE" ]; then
		SWAPSIZE=0
	fi


	REBOOTEACH=$(awk -F "=" '/REBOOTEACH/ {print $2}' $HACK_INI)
	if [ -z "$REBOOTEACH" ]; then
		REBOOTEACH=0
	fi
	
	TELNETD=$(awk -F "=" '/TELNETD/ {print $2}' $HACK_INI)

	RTSPSERVER=$(awk -F "=" '/RTSPSERVER/ {print $2}' $HACK_INI)

	SCHEDULE=$(awk -F "=" '/SCHEDULE/ {print $2}' $HACK_INI)
	MON=$(awk -F "=" '/MON/ {print $2}' $HACK_INI)
	TUE=$(awk -F "=" '/TUE/ {print $2}' $HACK_INI)
	WED=$(awk -F "=" '/WED/ {print $2}' $HACK_INI)
	THU=$(awk -F "=" '/THU/ {print $2}' $HACK_INI)
	FRI=$(awk -F "=" '/FRI/ {print $2}' $HACK_INI)
	SAT=$(awk -F "=" '/SAT/ {print $2}' $HACK_INI)
	SUN=$(awk -F "=" '/SUN/ {print $2}' $HACK_INI)

}


save_ini(){
	cp $SAVE_INI "${SAVE_INI}.bak"
	rm $SAVE_INI

	echo "ALARMREC=$ALARMREC" >> $SAVE_INI
	echo "FTPSERVER=$FTPSERVER" >> $SAVE_INI
	echo "FTPCLIENT=$FTPCLIENT" >> $SAVE_INI
	echo "FTPADDR=$FTPADDR" >> $SAVE_INI
	echo "FTPUSER=$FTPUSER" >> $SAVE_INI
	echo "FTPPASS=$FTPPASS" >> $SAVE_INI
	echo "FTPFOLDER=$FTPFOLDER" >> $SAVE_INI
	echo "FTPTRANSALARM=$FTPTRANSALARM" >> $SAVE_INI
	echo "FTPTRANSNORMAL=$FTPTRANSNORMAL" >> $SAVE_INI


	echo "SWAPSIZE=$SWAPSIZE" >> $SAVE_INI
	echo "FTPRECORD=$FTPRECORD"  >> $SAVE_INI
	echo "REBOOTEACH=$REBOOTEACH" >> $SAVE_INI
	echo "TELNETD=$TELNETD" >> $SAVE_INI
	echo "RTSPSERVER=$RTSPSERVER" >> $SAVE_INI
	echo "SCHEDULE=$SCHEDULE" >> $SAVE_INI
	echo "MON=$MON" >> $SAVE_INI
	echo "TUE=$TUE" >> $SAVE_INI
	echo "WED=$WED" >> $SAVE_INI
	echo "THU=$THU" >> $SAVE_INI
	echo "FRI=$FRI" >> $SAVE_INI
	echo "SAT=$SAT" >> $SAVE_INI
	echo "SUN=$SUN" >> $SAVE_INI
	echo "SAVED SUCCESS. "
}

#echo `export`

load_ini

if [ $REQUEST_METHOD = "POST" ]; then
	read param
	echo $param
	QUERY_STRING=$param
else
	echo ""
fi


LENGTH=`echo -e $QUERY_STRING | tr '&' '\n' | wc -l`


#echo $LENGTH
for i in `seq $LENGTH`
do
ELEMENT=`echo -e $QUERY_STRING | tr '&' '\t' |  cut -f $i`
#echo "<hr>"
#echo $ELEMENT

	KEY=`echo $ELEMENT |  cut -d "=" -f 1`
	VALUE=`echo $ELEMENT | cut -d "=" -f 2 | sed -e 's|%2f|/|ig'`
#	echo $KEY,$VALUE

	if [ "$KEY" = "mode" ]; then
		MODE=$VALUE

		if [ "$VALUE" = "set_ini" ]; then
			echo 
		fi
		
		if [ "$GROUP" = "FTP" ]; then
			FTPSERVER="off"
			FTPCLIENT="off"
			FTPTRANSNORMAL="off"
			FTPTRANSALARM="off"
		fi
		
		if [ "$GROUP" = "ALARM" ]; then
			ALARMREC="off"
		fi
		
		if [ "$GROUP" = "TELNET" ]; then
			TELNETD="off"
		fi
		
		if [ "$GROUP" = "RTSP" ]; then
			RTSPSERVER="off"
		fi
		
		if [ "$MODE" = "cmd_reboot" ]; then
			echo "REBOOTING NOW ......"
			echo "<a href=\"honeylab.cgi\"> RELOAD </a>"
			touch /tmp/cmd_reboot
		fi
				
		if [ "$MODE" = "cmd_clear_alarm" ]; then
			rm -rf /media/mmc/alarm_record
			echo "Complete."
			echo "<a href=\"honeylab.cgi\"> RELOAD </a>"
		fi
		
		if [ "$MODE" = "cmd_ftptest" ]; then
			touch /tmp/ftptest
			RETRY=0
			
			while true
			do
			
				if [ ! -f /tmp/ftptest ]; then
					break
				fi

				RETRY=`expr $RETRY + 1`
				if [ $RETRY == "10" ]; then
					echo "FTP Test is not completed in 10 sec"
					break
				fi
				sleep 1
			done

		fi
		
		if [ "$GROUP" = "SCHEDULE" ]; then
			echo "schedule"
			
			INISTR="000000000000000000000000"
			
			MON=$INISTR
			TUE=$INISTR
			WED=$INISTR
			THU=$INISTR
			FRI=$INISTR
			SAT=$INISTR
			SUN=$INISTR
			
		fi
	fi
	
	if [ "${KEY:0:3}" = "sch" ]; then
		DAY=${KEY:3:3}
#		echo "day=$DAY"
		HOUR=${KEY:6}
#		echo "hour=$HOUR"

#		echo "mon<br>"
		CMD="expr $HOUR + 1"
#		echo "CMD=$CMD"
		OFFSET=`$CMD`
#		echo "offset=$OFFSET"
		
		if [ "$DAY" = "MON" ]; then
			MON=${MON:0:$HOUR}1${MON:$OFFSET}
			echo "$MON<br>"
		fi
		if [ "$DAY" = "TUE" ]; then
			TUE=${TUE:0:$HOUR}1${TUE:$OFFSET}
			echo "$TUE<br>"
		fi
		if [ "$DAY" = "WED" ]; then
			WED=${WED:0:$HOUR}1${WED:$OFFSET}
			echo "$WED<br>"
		fi
		if [ "$DAY" = "THU" ]; then
			THU=${THU:0:$HOUR}1${THU:$OFFSET}
			echo "$THU<br>"
		fi
		if [ "$DAY" = "FRI" ]; then
			FRI=${FRI:0:$HOUR}1${FRI:$OFFSET}
			echo "$FRI<br>"
		fi
		if [ "$DAY" = "SAT" ]; then
			SAT=${SAT:0:$HOUR}1${SAT:$OFFSET}
			echo "$SAT<br>"
		fi
		if [ "$DAY" = "SUN" ]; then
			SUN=${SUN:0:$HOUR}1${SUN:$OFFSET}
			echo "$SUN<br>"
		fi
	fi

	if [ "$KEY" = "group" ]; then
		GROUP=$VALUE
	fi
	if [ "$GROUP" = "SWAP" ]; then
		if [ "$KEY" = "SWAPSIZE" ]; then
			SWAPSIZE=$VALUE
		fi
	fi

	if [ "$GROUP" = "TELNET" ]; then
		if [ "$KEY" = "TELNETD" ]; then
			TELNETD=$VALUE
		fi
	fi

	if [ "$GROUP" = "RTSP" ]; then
		if [ "$KEY" = "RTSPSERVER" ]; then
			RTSPSERVER=$VALUE
		fi
	fi
		
	if [ "$GROUP" = "ALARM" ]; then
		if [ "$KEY" = "ALARMREC" ]; then
			ALARMREC=$VALUE
		fi
		
	fi

	if [ "$GROUP" = "FTP" ]; then
	
		if [ "$KEY" = "FTPSERVER" ]; then
			FTPSERVER=$VALUE
		fi
		if [ "$KEY" = "FTPCLIENT" ]; then
			FTPCLIENT=$VALUE
		fi
		if [ "$KEY" = "FTPADDR" ]; then
			FTPADDR=$VALUE
		fi
		if [ "$KEY" = "FTPUSER" ]; then
			FTPUSER=$VALUE
		fi
		if [ "$KEY" = "FTPPASS" ]; then
			FTPPASS=$VALUE
		fi
		if [ "$KEY" = "FTPFOLDER" ]; then
			FTPFOLDER=$VALUE
		fi
		if [ "$KEY"= "FTPRECORD" ]; then
			FTPRECORD=$VALUE
		fi
		
		if [ "$KEY" = "FTPTRANSNORMAL" ]; then
			FTPTRANSNORMAL=$VALUE
		fi
		
		if [ "$KEY" = "FTPTRANSALARM" ]; then
			FTPTRANSALARM=$VALUE
		fi
	fi
	
	if [ "$GROUP" = "SCHEDULE" ]; then

		if [ "$KEY" = "SCHEDULE" ]; then
			SCHEDULE=$VALUE
		fi
	fi
	if [ "$GROUP" = "REBOOT" ]; then
		if [ "$KEY" = "REBOOTEACH" ]; then
			REBOOTEACH=$VALUE
		fi
	fi
done

if [ "$MODE" = "set_ini" ]; then

save_ini

echo "<a href=\"honeylab.cgi\"> return </a><br>";


echo "<textarea rows=\"10\" cols=\"120\">"

DATA=`cat $HACK_INI`
while read line
do
  echo $line
done << FILE
$DATA
FILE
echo "</textarea>"
echo "</form>"
echo "</html></body>"
exit;
fi

if [ "$FTPSERVER" = "on" ]; then
  F_FTPSERVER="checked"
else
  F_FTPSERVER=""
fi

if [ "$FTPCLIENT" = "on" ]; then
  F_FTPCLIENT="checked"
else
  F_FTPCLIENT=""
fi

if [ "$TELNETD" = "on" ]; then
  F_TELNETD="checked"
else
  F_TELNETD =""
fi


if [ "$RTSPSERVER" = "on" ]; then
  F_RTSPSERVER="checked"
else
  F_RTSPSERVER=""
fi


if [ "$ALARMREC" = "on" ]; then
	ALARMRECCHECK="checked"
else
	ALARMRECCHECK=""
fi

echo "<html><head><title>Custom rootfs settings</title></head><body>"
echo "<h1>ATOMCam Custom Rootfs Settings</h1>"
echo "<h2> Status </h2>"
echo "ATOMCam Application Version : "`awk -F "=" '/appver/ {print $2}' /atom/configs/app.ver`"<br>"
echo "ATOMCam MODEL : "`awk -F "=" '/PRODUCT_MODEL/ {print $2}' /atom/configs/.product_config`"<br>"
echo "System Time : "`TZ=JST-9 date`"<br>"
echo "Uptime : "`uptime`"<br>"
echo "Linux Kernel : "`cat /proc/version`"<br>"

echo "<h2>Alarm File Save</h2>"

echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"group\" value=\"ALARM\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"set_ini\">"
echo "<input type=\"checkbox\" name=\"ALARMREC\" $ALARMRECCHECK>Save Alarm MP4 into alarm_record folder (Note: No remove automatically.)<br>"
echo "<input type=\"submit\" value=\"Apply Alarm Setting\"><br>"
echo "</form>"

echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"cmd_clear_alarm\">"
echo "<input type=\"submit\" value=\"Clean Alarm Save Folder (/mmc/alarm_record))\"><br>"
echo "</form>"
echo "<hr>"

echo "<h2>FTP Setting</h2>"
echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"group\" value=\"FTP\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"set_ini\">"
echo "<input type=\"checkbox\" name=\"FTPSERVER\" $F_FTPSERVER >Enable FTP Server (Need to Reeboot)<br>"
echo "<input type=\"checkbox\" name=\"FTPCLIENT\" $F_FTPCLIENT >Working as FTP Client<br>"
echo "<table>"
echo "<tr><td>FTP Server IP Address</td>"
echo "<td><input type=\"textbox\" name=\"FTPADDR\" value=\"$FTPADDR\"></td></tr>"
echo "<tr><td>Login User as</td><td><input type=\"textbox\" name=\"FTPUSER\" value=\"$FTPUSER\"></td></tr>"
echo "<tr><td>Password</td><td><input type=\"textbox\" name=\"FTPPASS\" value=\"$FTPPASS\"></td></tr>"
echo "<tr><td>upload folder</td><td><input type=\"textbox\" name=\"FTPFOLDER\"  value=\"$FTPFOLDER\"></td></tr>"
echo "</table>"


if [ "$FTPTRANSNORMAL" = "on" ]; then
	ALLCHECK="checked"
else
	ALLCHECK=""
fi

if [ "$FTPTRANSALARM" = "on" ]; then
	ALARMCHECK="checked"
else
	ALARMCHECK=""
fi

echo "<input type=\"checkbox\" name=\"FTPTRANSNORMAL\" $ALLCHECK>ALL NORMAL RECORD FILES<br>"
echo "<input type=\"checkbox\" name=\"FTPTRANSALARM\"  $ALARMCHECK>ALARM RECORD FILES<br>"

echo "<input type=\"submit\" value=\"Apply FTP Setting\"><br>"
echo "</form>"


echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"cmd_ftptest\" target=\"_blank\">"
echo "<input type=\"submit\" value=\"FTP setting Test\"><br>"
echo "</form>"

if [ -f /tmp/log/ftptest.log ]; then
  echo "<textarea rows=\"10\" cols=\"120\">"
  DATA=`cat /tmp/log/ftptest.log `
while read line
do
  echo $line
done << FILE
$DATA
FILE
  echo "</textarea>"
  rm /tmp/log/ftptest.log
fi

if [ "$SCHEDULE" = "all" ]; then
	ALLREC="checked"
else
	ALLREC=""
fi

if [ "$SCHEDULE" = "scheduled" ]; then
	SCHEDULED="checked"
else
	SCHEDULED=""
fi

echo "<hr>"
echo "<h2>Schedule Recording</h2>"
echo "<form action=\"honeylab.cgi\" method=\"post\" name=\"scheduleform\">"
echo "<script>function allcheck( tf ) { var ElementsCount = document.scheduleform.elements.length; for( i=0 ; i<ElementsCount ; i++ ) { document.scheduleform.elements[i].checked = tf; }}</script>"
echo "<input type=\"hidden\" name=\"group\" value=\"SCHEDULE\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"set_ini\">"
echo "<input type=\"radio\" name=\"SCHEDULE\" value=\"all\" $ALLREC>ALL HOURS<br>"
echo "<input type=\"radio\" name=\"SCHEDULE\" value=\"scheduled\" $SCHEDULED>CUSTOM<br>"

echo "<table border=\"1\">"
echo "<tr>"

DWW=-1
for d in "---" "SUN" "MON" "TUE" "WED" "THU" "FRI" "SAT"
do
	DW=`TZ=JST-9 date "+%w"`
	HOUR=`TZ=JST-9 date "+%H"`
	if [ $DWW == $DW ]; then
		echo "<td><b>$d</b></td>"
	else
		echo "<td>$d</td>"
	fi
	DWW=`expr $DWW + 1`

	for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
	do
		if [ $d == "---" ]; then
			if [ `expr $HOUR + 0` ==  `expr $i + 0` ]; then
				echo "<td><b>$i</b></td>"
			else
				echo "<td>$i</td>"
			fi
		else
			if [ $d == "MON" ]; then
				FLAG=${MON:0:1}
				MON=${MON:1}
			fi
			if [ $d == "TUE" ]; then
				FLAG=${TUE:0:1}
				TUE=${TUE:1}
			fi
			if [ $d == "WED" ]; then
				FLAG=${WED:0:1}
				WED=${WED:1}
			fi
			if [ $d == "THU" ]; then
				FLAG=${THU:0:1}
				THU=${THU:1}
			fi
			if [ $d == "FRI" ]; then
				FLAG=${FRI:0:1}
				FRI=${FRI:1}
			fi
			if [ $d == "SAT" ]; then
				FLAG=${SAT:0:1}
				SAT=${SAT:1}
			fi			
			if [ $d == "SUN" ]; then
				FLAG=${SUN:0:1}
				SUN=${SUN:1}
			fi			

			SCK=""
			if [ "$FLAG" == "1" ]; then
				SCK="checked"
			fi
			
			echo "<td><input type=\"checkbox\" name=\"sch$d$i\" $SCK></td>"
		fi
	done
echo "</tr>"
done

echo "</table>"
echo "<input type=\"button\" name="sch_all" value=\"ALL\" onclick=\"allcheck(true);\">"
echo "<input type=\"button\" name="sch_clear" value=\"clear\" onclick=\"allcheck(false);\">"

echo "<input type=\"submit\" value=\"Apply Schedule Setting\"><br>"
echo "</form>"

echo "<hr>"
echo "<h2>Swap file on SD Card</h2>"
echo "Swap file size"
echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"group\" value=\"SWAP\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"set_ini\">"
echo "<input type=\"textbox\" value=\"$SWAPSIZE\" name=\"SWAPSIZE\"> MB ( 0 = disabled)"
echo "<input type=\"submit\" value=\"Apply Swap Setting\"> (Need to reboot)<br>"
echo "</form>"
echo "<hr>"
echo "<h2>Reboot Setting</h2>"


echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"group\" value=\"REBOOT\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"set_ini\">"
echo "Reboot in each <input type=\"textbox\" value=\"$REBOOTEACH\" name=\"REBOOTEACH\"> hours.( 0 = disabled)"
echo "<input type=\"submit\" value=\"Apply Reboot setting\"><br>"
echo "</form>"

echo "<hr>"
echo "<h2>Telnetd setting</h2>"
echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"group\" value=\"TELNET\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"set_ini\">"
echo "<input type=\"checkbox\" name=\"TELNETD\" $F_TELNETD> USE Telnetd"
echo "<input type=\"submit\" value=\"Apply Telnetd Setting\">(Need to reboot)<br>"

IPADDR=${HTTP_HOST%:*}

echo "<hr>"
echo "<h2>RTSP setting (not recommended)</h2>"
echo " !!!! MUST Set swap file > 40MB !!!! <br>" 
echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"group\" value=\"RTSP\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"set_ini\">"
echo "<input type=\"checkbox\" name=\"RTSPSERVER\" $F_RTSPSERVER> USE RTSP Streaming"
echo "<input type=\"submit\" value=\"Apply RTSP Setting\">(Need to reboot)<br>"


echo "streaming URL :  <input type=\"textbox\" size=\"64\" value=\"rtsp://$IPADDR:8554/unicast\"><br>"
RTSPWORKING=`ps -w | grep 'v4l2rtspserver' | grep -v 'grep' | wc -l`


echo "Working state : "
if [ $RTSPWORKING = "0" ]; then
	echo "Stopping"
else
	echo "Runnning"
fi

echo "</form>"

echo "<hr>"
echo "<h2>Maintenance</h2>"
echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"cmd_reboot\" target=\"_blank\">"
echo "<input type=\"submit\" value=\"DO REBOOT NOW\"><br>"
echo "</form>"

echo "<form action=\"honeylab.cgi\" method=\"post\">"
echo "<input type=\"hidden\" name=\"mode\" value=\"cmd_clear_alarm\">"
echo "<input type=\"submit\" value=\"Clean Alarm Save Folder (/mmc/alarm_record))\"><br>"
echo "</form>"

echo "<hr>"
echo "<h2>System Information</h2>"

echo "<textarea cols=\"120\" rows=\"10\">"

DATA=`echo -e ">/proc/version\n "`
DATA=$DATA`cat /proc/version`
DATA=$DATA`echo -e "\n------------------\n "`
DATA=$DATA`echo -e ">df\n "`

DATA=`echo -e ">free\n "`
DATA=$DATA`free`
DATA=$DATA`echo -e "\n------------------\n "`
DATA=$DATA`echo -e ">df\n "`
DATA=$DATA`df`

DATA=$DATA`echo -e "\n------------------\n "`
DATA=$DATA`echo -e ">mount\n "`
DATA=$DATA`mount`


DATA=$DATA`echo -e "\n------------------\n "`
DATA=$DATA`echo -e ">netstat -a\n "`
DATA=$DATA`netstat -a 2> /dev/null`

while read line
do
	L=`echo -e "${line// /&nbsp;}"`
	echo -e $L
done << FILE
$DATA
FILE

echo "</textarea>"

echo "<hr>"

echo "<hr>"
