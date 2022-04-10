#!/bin/sh

HACK_INI=/tmp/hack.ini
HEALTHCHECK_PING_URL=$(awk -F "=" '/HEALTHCHECK_PING_URL *=/ {print $2}' $HACK_INI)
[ "$HEALTHCHECK_PING_URL" != "" ] && echo $(TZ=JST-9 date +"%Y/%m/%d %H:%M:%S : ") `curl -fsS -m 10 --retry 5 $HEALTHCHECK_PING_URL` >> /media/mmc/healthcheck.log
