#!/bin/sh

echo "Cache-Control: no-cache"
echo "Content-Type: plain/text"
echo ""
echo TIMESTAMP=`TZ=JST-9 date +"%Y/%m/%d %X"`
