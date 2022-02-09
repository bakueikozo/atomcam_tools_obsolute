#!/bin/sh

res=`/scripts/cmd jpeg`

echo "Cache-Control: no-cache"
echo "Content-Type: image/jpeg"
echo ""
cat /tmp/snapshot.jpg
