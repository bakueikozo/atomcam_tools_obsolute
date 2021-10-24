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
insmod /system/driver/rtl8189ftv.ko
insmod /system/driver/speaker_ctl.ko

devmem 0x10011110 32 0x6e094800
devmem 0x10011138 32 0x300
devmem 0x10011134 32 0x300
touch /tmp/resolv.conf
mkdir -p /tmp/log

/system/bin/ver-comp
/system/bin/assis >> /tmp/log/assis.log 2>&1 &

/system/bin/hl_client >> /tmp/log/hl_client.log 2>&1 &

if [ "$1" = "on" ]; then
  LD_PRELOAD=/tmp/system/modules/libcallback.so /system/bin/iCamera_app >> /tmp/log/atom.log 2>&1 &
else
  /system/bin/iCamera_app >> /tmp/log/atom.log 2>&1 &
fi

