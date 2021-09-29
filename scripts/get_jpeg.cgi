#!/bin/sh

echo "Cache-Control: no-cache"
echo "Content-Type: image/jpeg"
echo ""


touch /tmp/get_jpeg
while true
do
	if [ ! -f /tmp/get_jpeg ]; then
		break
	fi
done

cat /tmp/snapshot.jpg

