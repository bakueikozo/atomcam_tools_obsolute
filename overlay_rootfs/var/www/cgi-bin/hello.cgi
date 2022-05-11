#!/bin/sh

echo "Content-type: text/html"
echo ""
echo "<html>"
echo "<head><title>CGI Output</title></head>"
echo "<body>"

QUERY1=${QUERY_STRING##name=}
QUERY2=${QUERY1%%&*}
SPATH="/var/www/SDPath/"${QUERY2%/}
for i in `ls ${SPATH}`
do
  if [ -f "${SPATH}/$i" ]; then
    echo "<h1>$i</h1>"
  fi
  if [ -d "${SPATH}/$i" ]; then
    echo "<h2>$i</h2>"
  fi
done

echo "</body>"
echo "</html>"
