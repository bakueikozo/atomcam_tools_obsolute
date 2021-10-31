#!/bin/sh

echo "Cache-Control: no-cache"
echo "Content-Type: plain/text"
echo ""

if [ "$REQUEST_METHOD" = "POST" ]; then
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
  ' >> /var/run/webcmd
  read ack < /var/run/webres
  echo $ack
fi
