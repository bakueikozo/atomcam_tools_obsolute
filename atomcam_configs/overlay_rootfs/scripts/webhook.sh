#!/bin/sh

HACK_INI=/media/mmc/hack.ini
tail -F /tmp/log/atom.log | awk '
BEGIN {
  FS = "=";
  while(getline < HACK_INI) {
    ENV[$1]=$2;
  }
  if(ENV["WEBHOOK"] != "on") exit;
  if(ENV["WEBHOOK_URL"] == "") exit;
}

/alarm_uploadNotify/ {
  if(ENV["WEBHOOK_ALARM_EVENT"] == "on") Post("alarmEvent");
}
/\[curl_post.*?\]snd:/ {
  gsub(/^.*snd:\[/,"");
  gsub(/\]$/, "");
  if(ENV["WEBHOOK_ALARM_INFO"] == "on") Post("recognitionNotify", $0);
}
/upload video succes!/ {
  if(ENV["WEBHOOK_ALARM_VIDEO_FINISH"] == "on") Post("uploadVideoFinish");
}
/upload pic succes!/ {
  if(ENV["WEBHOOK_ALARM_PICT_FINISH"] == "on") Post("uploadPictureFinish");
}
/\[exec-iCame,.* cmd:\[mv/ {
  gsub(/^.*\/media\/mmc/, "");
  gsub(/;sync;.*$/, "");
  if(ENV["WEBHOOK_RECORD_EVENT"] == "on") Post("recordEvent", "\"" $0 "\"");
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
    system("curl -X POST -H \x27Content-Type: application/json\x27 -d \x27{\"type\":\"" event "\"}\x27 " ENV["WEBHOOK_URL"]);
  } else {
    system("curl -X POST -H \x27Content-Type: application/json\x27 -d \x27{\"type\":\"" event "\", \"data\":" data "}\x27 " ENV["WEBHOOK_URL"]);
  }
}
' -v HACK_INI=$HACK_INI
