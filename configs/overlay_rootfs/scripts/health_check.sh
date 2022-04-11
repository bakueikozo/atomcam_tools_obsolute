#!/bin/sh

HACK_INI=/tmp/hack.ini
HEALTHCHECK_PING_URL=$(awk -F "=" '/HEALTHCHECK_PING_URL *=/ {print $2}' $HACK_INI)
[ "$HEALTHCHECK_PING_URL" != "" ] && echo $(TZ=JST-9 date +"%Y/%m/%d %H:%M:%S : ") `curl --ipv4 --location --silent --show-error --output /dev/null --write-out "%{http_code}" $HEALTHCHECK_PING_URL` >> /media/mmc/healthcheck.log
