#!/bin/sh
HACK_INI=/media/mmc/hack.ini
REBOOT=$(awk -F "=" '/REBOOT *=/ {print $2}' $HACK_INI)

if [ "$REBOOT" = "on" ]; then
  sync
  sync
  sync
  reboot
fi
