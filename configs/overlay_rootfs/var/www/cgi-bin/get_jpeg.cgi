#!/bin/sh

TFILE=/tmp/get_jpeg_${QUERY_STRING##=}
touch /tmp/get_jpeg
touch ${TFILE}
while [ -f /tmp/get_jpeg ]
do
  if [ /tmp/get_jpeg -nt ${TFILE} ] ; then
    rm -f ${TFILE}
    exit -1
  fi
  sleep 0.2
done
rm -f ${TFILE}

echo "Cache-Control: no-cache"
echo "Content-Type: image/jpeg"
echo ""
cat /tmp/snapshot.jpg
