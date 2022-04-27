#!/bin/sh

TZ=JST-9 date >> /media/mmc/atomhack.log
free >> /media/mmc/atomhack.log
df -k | grep ' /tmp$' >> /media/mmc/atomhack.log
