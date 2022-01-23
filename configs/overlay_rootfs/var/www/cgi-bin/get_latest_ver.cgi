#!/bin/sh

echo "Cache-Control: no-cache"
echo "Content-Type: plain/text"
echo ""

latest=`curl -o - https://github.com/mnakada/atomcam_tools/releases/latest`
latest=${latest##*Ver.}
latest=${latest%%\"*}
echo LATESTVER=${latest}
