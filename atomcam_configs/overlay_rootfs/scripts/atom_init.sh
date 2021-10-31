#!/bin/sh

export PATH=/tmp/system/bin:/system/bin:/bin:/sbin:/usr/bin:/usr/sbin
export LD_LIBRARY_PATH=/thirdlib:/system/lib:/tmp
insmod /system/driver/tx-isp-t31.ko isp_clk=100000000
insmod /system/driver/exfat.ko
insmod /system/driver/audio.ko spk_gpio=-1
insmod /system/driver/avpu.ko
insmod /system/driver/sinfo.ko
insmod /system/driver/sample_pwm_core.ko
insmod /system/driver/sample_pwm_hal.ko
insmod /system/driver/mmc_detect_test.ko || VENDERID="0x024c"
insmod /system/driver/speaker_ctl.ko

if [ "$VENDERID" = "" ]; then
  while [ ! -f /sys/bus/mmc/devices/mmc1\:0001/mmc1\:0001\:1/vendor ]; do
	  sleep 0.1
  done
  VENDERID=`cat /sys/bus/mmc/devices/mmc1\:0001/mmc1\:0001\:1/vendor`
fi
if [ "0x024c" = "$VENDERID" ]; then
	insmod /system/driver/rtl8189ftv.ko
elif [ "0x007a" = "$VENDERID" ]; then
	insmod /thirdlib/atbm603x_wifi_sdio.ko
fi

devmem 0x10011110 32 0x6e094800
devmem 0x10011138 32 0x300
devmem 0x10011134 32 0x300
touch /tmp/resolv.conf
mkdir -p /tmp/log
chmod 777 /tmp/log

/system/bin/ver-comp
/system/bin/assis >> /tmp/log/assis.log 2>&1 &

/system/bin/hl_client >> /tmp/log/hl_client.log 2>&1 &

LD_PRELOAD=/tmp/system/modules/libcallback.so /system/bin/iCamera_app >> /tmp/log/atom.log 2>&1 &

/system/bin/dongle_app >> /tmp/log/dongle.log &
