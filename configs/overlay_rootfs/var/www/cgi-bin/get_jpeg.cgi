#!/bin/sh

res=`/scripts/cmd jpeg`
[ "$res" = "error" ] && exit 1

echo "Cache-Control: no-cache"
echo "Content-Type: image/jpeg"
echo ""
cat /tmp/snapshot.jpg
