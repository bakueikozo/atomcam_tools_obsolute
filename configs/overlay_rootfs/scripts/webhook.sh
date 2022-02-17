#!/bin/sh

HACK_INI=/tmp/hack.ini
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
  gsub(/^.*Predict \[off:[0-9]*\] /, "");
  gsub(/tm:/, "");
  gsub(/\|/, ",");
  gsub(/res:/, ",");
  gsub(/\[/, "");
  gsub(/\]/, "");
  if(ENV["WEBHOOK_ALARM_INFO"] == "on") Post("recognitionNotify", "\"" $0 "\"");
}
/\[webhook\] time_lapse_event/ {
  gsub(/^.*time_lapse_event /, "");
  if(ENV["WEBHOOK_TIMELAPSE_EVENT"] == "on") Post("timelapseEvent", "\"" $0 "\"");
}
/\[webhook\] time_lapse_finish/ {
  gsub(/^.*time_lapse_finish /, "");
  if(ENV["WEBHOOK_TIMELAPSE_FINISH"] == "on") Post("timelapseFinish", "\"" $0 "\"");
}

function Post(event, data) {
  if(data == "") {
    system("curl -X POST -H \x27Content-Type: application/json\x27 -d \x27{\"type\":\"" event "\", \"device\":\"" HOSTNAME "\"}\x27 " ENV["WEBHOOK_URL"] " > /dev/null 2>&1");
  } else {
    system("curl -X POST -H \x27Content-Type: application/json\x27 -d \x27{\"type\":\"" event "\", \"device\":\"" HOSTNAME "\", \"data\":" data "}\x27 " ENV["WEBHOOK_URL"] " > /dev/null 2>&1");
  }
}
' -v HACK_INI=$HACK_INI
