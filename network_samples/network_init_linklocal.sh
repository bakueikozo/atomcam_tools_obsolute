#!/bin/sh
# Place it in the top-directory of the SD-Card with the name network_init.sh.

case "$1" in
  start)
    devmem 0x10000040 32 0x0b000FFF

    modprobe g_ncm
    modprobe rndis_host

    count=0
    while ! ip link | grep usb0 > /dev/null
    do
      sleep 0.5
      let count++
      [ 20 -le $count ] && break
    done

    ifconfig usb0 up
    /usr/sbin/avahi-autoipd -D --no-drop-root usb0

    count=0
    while ! ifconfig usb0:avahi | grep 'inet addr' > /dev/null
    do
      sleep 0.5
      let count++
      [ 20 -le $count ] && exit 1 # fallback to WiFi
    done

    exit 0
  ;;
  restart)
    ifconfig usb0:avahi down
    ifconfig usb0 down
    ifconfig usb0 up
    /usr/sbin/avahi-autoipd -D --no-drop-root usb0

    exit 0
  ;;
  *)
    echo "Usage: $0 (start|restart)"
    exit 1
esac

exit $?
