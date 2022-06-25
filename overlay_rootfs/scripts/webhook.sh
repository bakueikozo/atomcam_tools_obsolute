#!/bin/sh

HACK_INI=/tmp/hack.ini
mkdir -p /tmp/log
awk '
BEGIN {
  FS = "=";
  while(getline < HACK_INI) {
    ENV[$1]=$2;
  }
  "hostname" | getline HOSTNAME;
  lastTimestamp = 0;
  logDisable = 0;
}

/\[webhook\] time_lapse_finish/ {
  split($0, str, / \t*/);
  system("/scripts/timelapse_finish.sh " str[3]);
}

{
  timestamp = systime();
  logLength += length($0);
  if(timestamp != lastTimestamp) {
    if(logLength / (timestamp - lastTimestamp) < 1024) {
      logDisable = 0;
    } else {
      logDisable = 1;
      time = strftime("%Y/%m/%d %H:%M:%S", timestamp);
      printf("%s : --- Logging is suspended ---\n", time) >> "/tmp/log/atom.log";
    }
    logLength = 0;
    lastTimestamp = timestamp;
  }
  if(!logDisable) print >> "/tmp/log/atom.log";
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

function Post(event, data) {
  if(data == "") {
    system("curl -X POST -m 3 -H \x27Content-Type: application/json\x27 -d \x27{\"type\":\"" event "\", \"device\":\"" HOSTNAME "\"}\x27 " ENV["WEBHOOK_URL"] " > /dev/null 2>&1");
  } else {
    system("curl -X POST -m 3 -H \x27Content-Type: application/json\x27 -d \x27{\"type\":\"" event "\", \"device\":\"" HOSTNAME "\", \"data\":" data "}\x27 " ENV["WEBHOOK_URL"] " > /dev/null 2>&1");
  }
}
' -v HACK_INI=$HACK_INI /var/run/atomapp
