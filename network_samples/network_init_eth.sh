#!/bin/sh
# Place it in the top-directory of the SD-Card with the name network_init.sh.
# Not tested due to lack of environment.

PRODUCT_CONFIG=/atom/configs/.product_config
PRODUCT_MODEL=$(awk -F "=" '/PRODUCT_MODEL *=/ {print $2}' $PRODUCT_CONFIG)

case "$1" in
  start)
    if [ "$PRODUCT_MODEL" = "AC1" ] ; then
      echo 47 > /sys/class/gpio/export
      echo out > /sys/class/gpio/gpio47/direction
      echo 0 > /sys/class/gpio/gpio47/active_low
      echo 1 > /sys/class/gpio/gpio47/value
    fi
    modprobe r8152
    modprobe asix
    modprobe ax88179_178a
    modprobe cdc_ether

    count=0
    while ! ip link | grep eth0 > /dev/null
    do
      sleep 0.5
      let count++
      [ 20 -le $count ] && break
    done

    ifconfig eth0 up
    udhcpc -i eth0 -x hostname:ATOM -p /var/run/udhcpc.pid -b

    count=0
    while ! ifconfig eth0 | grep 'inet addr' > /dev/null
    do
      sleep 0.5
      let count++
      [ 20 -le $count ] && exit 1 # fallback to WiFi
    done

    exit 0
  ;;
  restart)
    ifconfig eth0 down
    ifconfig eth0 up
    killall -USR1 udhcpc
    ps | grep -v grep | grep udhcpc || udhcpc -i eth0 -x hostname:ATOM -p /var/run/udhcpc.pid -b

    exit 0
  ;;
  *)
    echo "Usage: $0 (start|restart)"
    exit 1
esac

exit $?
