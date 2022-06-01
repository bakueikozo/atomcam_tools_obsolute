#!/bin/sh
[ "$1" = "wpa_supplicant" ] && exit 0
[ "$1" = "udhcpc" ] && exit 0
/bin/busybox ${0##*/} $*
