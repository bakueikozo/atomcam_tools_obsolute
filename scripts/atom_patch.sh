#!/bin/sh
echo "$0 $*" >> /tmp/log/${0##*/}.log
LD_LIBRARY_PATH=/atom/thirdlib:/atom/system/lib:/atom/lib $0_org $*
