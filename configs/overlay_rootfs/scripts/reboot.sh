#!/bin/sh
HACK_INI=/media/mmc/hack.ini
REBOOT=$(awk -F "=" '/REBOOT *=/ {print $2}' $HACK_INI)

if [ "$REBOOT" = "on" ]; then
  killall -SIGUSR2 iCamera_app
  sync
  sync
  sync
  reboot
fi
