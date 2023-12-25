#!/bin/bash
set -e

cd /atomtools/build/buildroot-2016.02

rm -rf /atomtools/build/buildroot-2016.02/package/ffmpeg
rm -rf /atomtools/build/buildroot-2016.02/package/mjpg-streamer
rm -rf /atomtools/build/buildroot-2016.02/package/ncurses
cp -r /src/custompackages/package/* /atomtools/build/buildroot-2016.02/package/

patch -p1 < /src/patches/add_fp_no_fused_madd.patch
patch -p1 < /src/patches/libv4l_add_lpthread.patch
patch -p1 < /src/patches/linux_makefile.patch

cp /src/configs/atomcam_defconfig configs/
cp /src/configs/busybox.config package/busybox
make atomcam_defconfig

# mipsel-gcc for uLibc
cd /atomtools/build
if [ ! -d mips-gcc472-glibc216-64bit ] ; then
  git clone https://github.com/Dafang-Hacks/mips-gcc472-glibc216-64bit.git --depth 1
  cd mips-gcc472-glibc216-64bit
  patch -p1 < /src/patches/linux_uclibc_hevc.patch
fi

# Start the build process
cd /atomtools/build/buildroot-2016.02
make clean
make host-uboot-tools
make dosfstools-init
for i in `make show-targets`
do
  echo "=== $i ==="
  make $i || {
    echo "XXXX $i XXXX"
    break;
  }
done
# toolchain-buildroot toolchain alsa-lib alsa-utils atbm_wifi avahi bash busybox cifs-utils dbus dosfstools-init e2fsprogs host-e2fsprogs expat ffmpeg file findutils freetype fnext2fs glibc h264bitstream htop ingenic_videocap initscripts jpeg lame libcurl libdaemon libevent libffi libjpeg libnl lighttpd linux-headers live555 log4cpp logrotate lsof micropython-lib micropython host-mke2img host-mtd nano ncurses nss-mdns openssh openssl host-patchelf pcre popt python readline rtl8189es_linux samba4 skeleton sudo host-uboot-tools util-linux v4l2cpp v4l2loopback v4l2rtspserver wpa_supplicant xz zlib linux host-mke2img host-fakeroot host-makedevs host-mkpasswd rootfs-ext2

