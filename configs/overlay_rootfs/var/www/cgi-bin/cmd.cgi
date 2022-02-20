#!/bin/sh

echo "Cache-Control: no-cache"
echo "Content-Type: plain/text"
echo ""

if [ "$REQUEST_METHOD" = "GET" ]; then
  NAME=${QUERY_STRING##name=}
  if [ "$NAME" = "" -o "$NAME" = "latest-ver" ] ; then
    latest=`curl -o - https://github.com/mnakada/atomcam_tools/releases/latest`
    latest=${latest##*Ver.}
    latest=${latest%%\"*}
    echo LATESTVER=${latest}
  fi
  if [ "$NAME" = "" -o "$NAME" = "time" ] ; then
    echo TIMESTAMP=`TZ=JST-9 date +"%Y/%m/%d %X"`
  fi
  if [ "$NAME" = "" -o "$NAME" = "move" ] ; then
    echo MOTORPOS=`echo move | nc localhost:4000`
  fi
fi

if [ "$REQUEST_METHOD" = "POST" ]; then
  PORT=${QUERY_STRING##port=}
  awk '
    BEGIN {
      RS="[{},]";
    }
    /^$/ { next; }
    /\"exec\":\"/ {
      gsub(/^[ \t]*\"exec\":\"/, "");
      gsub(/\"[ \t]*$/, "");
      print $0;
      fflush();
    }
  ' | (
    if [ "$PORT" = "socket" ]; then
      /usr/bin/nc localhost:4000
    else
      cat >> /var/run/webcmd
      read ack < /var/run/webres
      echo $ack
    fi
  )
fi
