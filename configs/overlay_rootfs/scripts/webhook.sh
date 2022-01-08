#!/bin/sh

HACK_INI=/media/mmc/hack.ini
touch $HACK_INI
mkdir -p /tmp/log
touch /tmp/log/atom.log
tail -F /tmp/log/atom.log | TZ=JST-9 awk '
BEGIN {
  FS = "=";
  while(getline < HACK_INI) {
    ENV[$1]=$2;
  }
  "hostname" | getline HOSTNAME;
}

/__NTP Set SysTime To/ {
  gsub(/^.*__NTP Set SysTime To /, "");
  gsub(/__.*$/, "000");
  print strftime("Reboot Time : %Y/%m/%d %H:%M:%S") >> "/media/mmc/atomhack.log";
  fflush("/media/mmc/atomhack.log");
}

{
  if(ENV["WEBHOOK"] != "on") next;
  if(ENV["WEBHOOK_URL"] == "") next;
}

/\[aiAlgo\] start/ {
  if(ENV["WEBHOOK_ALARM_EVENT"] == "on") Post("alarmEvent");
}
/\[aiAlgo\] call_TD_Human_Pet_Predict/ {
  gsub(/^.*Predict \[off:\d*\] /, "");
  gsub(/tm:/, '');
  gsub(/|/, ",");
  gsub(/res:/, ",");
  gsub(/[\[\]]/, "");
  if(ENV["WEBHOOK_ALARM_INFO"] == "on") Post("recognitionNotify", $0);
}
/time_lapse_exec_cur_task/ {
  gsub(/^.*seq: /, "");
  if(ENV["WEBHOOK_TIMELAPSE_EVENT"] == "on") Post("timelapseCurrent", "\"" $0 "\"");
}
/time_lapse_stop_job/ {
  if(ENV["WEBHOOK_TIMELAPSE_FINISH"] == "on") Post("timelapseFinish");
}

function Post(event, data) {
  if(data == "") {
    system("curl -X POST -H \x27Content-Type: application/json\x27 -d \x27{\"type\":\"" event "\", \"device\":\"" HOSTNAME "\"}\x27 " ENV["WEBHOOK_URL"]);
  } else {
    system("curl -X POST -H \x27Content-Type: application/json\x27 -d \x27{\"type\":\"" event "\", \"device\":\"" HOSTNAME "\", \"data\":" data "}\x27 " ENV["WEBHOOK_URL"]);
  }
}
' -v HACK_INI=$HACK_INI
