#!/bin/sh

export PATH=/tmp/system/bin:/system/bin:/bin:/sbin:/usr/bin:/usr/sbin
export LD_LIBRARY_PATH=/thirdlib:/system/lib:/tmp
PRODUCT_CONFIG=/configs/.product_config
PRODUCT_MODEL=$(awk -F "=" '/PRODUCT_MODEL *=/ {print $2}' $PRODUCT_CONFIG)
APPVER_FILE=/configs/app.ver
APPVER=$(awk -F "=" '/appver *=/ {print $2}' $APPVER_FILE)

sleep 1

insmod /system/driver/tx-isp-t31.ko isp_clk=100000000
insmod /system/driver/exfat.ko
if [ "ATOM_CAKP1JZJP" = "$PRODUCT_MODEL" ] ; then
  insmod /system/driver/audio.ko spk_gpio=-1 alc_mode=0 mic_gain=0
else
  insmod /system/driver/audio.ko spk_gpio=-1
fi
insmod /system/driver/avpu.ko
insmod /system/driver/sinfo.ko
insmod /system/driver/sample_pwm_core.ko
insmod /system/driver/sample_pwm_hal.ko
insmod /system/driver/speaker_ctl.ko

devmem 0x10011110 32 0x6e094800
devmem 0x10011138 32 0x300
devmem 0x10011134 32 0x300

VENDERID="0x024c"
if [ -f /system/driver/mmc_detect_test.ko ]; then
  insmod /system/driver/mmc_detect_test.ko
  while [ ! -f /sys/bus/mmc/devices/mmc1\:0001/mmc1\:0001\:1/vendor ]; do
    sleep 0.1
  done
  VENDERID=`cat /sys/bus/mmc/devices/mmc1\:0001/mmc1\:0001\:1/vendor`
fi
if [ "0x024c" = "$VENDERID" ]; then
  insmod /system/driver/rtl8189ftv.ko
elif [ "0x007a" = "$VENDERID" ]; then
  [ -f /usr/share/atbm603x_conf/atbm_txpwer_dcxo_cfg.txt ] && cp /usr/share/atbm603x_conf/atbm_txpwer_dcxo_cfg.txt /tmp
  [ -f /usr/share/atbm603x_conf/set_rate_power.txt ] && cp /usr/share/atbm603x_conf/set_rate_power.txt /tmp
  [ -f /thirdlib/atbm603x_wifi_sdio.ko ] && insmod /thirdlib/atbm603x_wifi_sdio.ko
  [ -f /system/driver/atbm603x_wifi_sdio.ko ] && insmod /system/driver/atbm603x_wifi_sdio.ko
  sleep 1
  if [ ! -f /sys/module/atbm603x_wifi_sdio/parameters/fw_ver ]; then
    sync
    echo 3 > /proc/sys/vm/drop_caches
    [ -f /thirdlib/atbm603x_wifi_sdio.ko ] && insmod /thirdlib/atbm603x_wifi_sdio.ko
    [ -f /system/driver/atbm603x_wifi_sdio.ko ] && insmod /system/driver/atbm603x_wifi_sdio.ko
  fi
elif [ "0x5653" = "$VENDERID" ]; then
  insmod /system/driver/ssv6x5x.ko stacfgpath=/system/driver/ssv6x5x-wifi.cfg
elif [ "0x424c" = "$VENDERID" ]; then
    insmod /system/driver/bl_fdrv.ko
fi

mkdir -p /tmp/app
touch /tmp/resolv.conf
mkdir -p /tmp/log
chmod 777 /tmp/log

sed -i.old -e 's/^alarmInterval=.*$/alarmInterval=30/' /configs/.user_config

/system/bin/ver-comp
/system/bin/assis >> /tmp/log/assis.log 2>&1 &

[ "ATOM_CAKP1JZJP" = "$PRODUCT_MODEL" ] && insmod /system/driver/sample_motor.ko vstep_offset=0 hmaxstep=2130 vmaxstep=1580

/system/bin/hl_client >> /tmp/log/hl_client.log 2>&1 &

(LD_PRELOAD=/tmp/system/modules/libcallback.so /system/bin/iCamera_app | awk '/^\[nas\.c/ { next; } { print; fflush(); }' ) >> /tmp/log/atom.log 2>&1 &

[ "AC1" = "$PRODUCT_MODEL" ] && /system/bin/dongle_app >> /tmp/log/dongle.log &

(
while sleep 3600
do
  TZ=JST-9 date >> /media/mmc/atomhack.log
  free >> /media/mmc/atomhack.log
done
) &
