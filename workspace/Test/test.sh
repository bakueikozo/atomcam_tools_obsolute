#!/bin/sh

#ext2 mount
insmod /media/mmc/modules/crc16.ko
insmod /media/mmc/modules/mbcache.ko
insmod /media/mmc/modules/jbd2.ko
insmod /media/mmc/modules/ext4.ko
mkdir /tmp/newroot
mount -t ext2 /media/mmc/rootfs_hack.ext2 /tmp/newroot

# remount
mkdir -p /tmp/newroot/atom
cd /tmp/newroot
/tmp/Test/busybox pivot_root . atom
cd /
rmdir /tmp/newroot
mount --move /atom/proc /proc
mount --move /atom/dev /dev
mount --move /atom/sys /sys
mount --move /atom/tmp /tmp
mount --move /atom/media /media

mkdir -p /tmp/log
/media/mmc/scripts/honeylab_init.sh atom_mounted > /tmp/log/honeylab_init.log 2>&1 &
