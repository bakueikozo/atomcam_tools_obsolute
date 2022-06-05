#!/bin/sh
# Place it in the top-directory of the SD-Card with the name network_init.sh.
# Not tested due to lack of environment.

case "$1" in
  start)
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
    udhcpc -i eth0 -x hostname:ATOM -p /var/run/udhcpc.pid -b &

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
    killall -USR1 udhcpc || udhcpc -i eth0 -x hostname:ATOM -p /var/run/udhcpc.pid -b >> /media/mmc/healthcheck.log 2>&1

    exit 0
  ;;
  *)
    echo "Usage: $0 (start|restart)"
    exit 1
esac

exit $?
