#!/bin/sh

set -x
echo '\n------------test.sh------------'

echo "mount mmc to /tmp/mmc"

mkdir /tmp/mmc
mount /dev/mmcblk0p1 /tmp/mmc

echo '\n----------- calling honeylab_init ---'
/tmp/mmc/scripts/honeylab_init.sh

