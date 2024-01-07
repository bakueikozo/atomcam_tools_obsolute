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
CROSS_TOOLS=crosstool-ng-1.26.0
useradd -m cross
mkdir -p /atomtools/build/cross/mips-uclibc
mkdir -p /atomtools/build/cross/src
mkdir -p /atomtools/build/cross/src/work
chown -R cross:cross /atomtools/build/cross
cd /atomtools/build/cross/src
curl http://crosstool-ng.org/download/crosstool-ng/${CROSS_TOOLS}.tar.xz | tar Jxvf -
cd ${CROSS_TOOLS}
./configure --prefix=/atomtools/build/cross/tools
make
make install

cd /atomtools/build/cross/src/work
cp /src/configs/crosstools_config .config
chown cross:cross .config
sudo -u cross /atomtools/build/cross/tools/bin/ct-ng build

cd /atomtools/build/cross/mips-uclibc/mipsel-ingenic-linux-uclibc/sysroot
patch -p1 < /src/patches/linux_uclibc_hevc.patch

# Start the build process
cd /atomtools/build/buildroot-2016.02
make clean
for i in host-uboot-tools dosfstools-init `make show-targets`
do
  echo `date '+%H:%M:%S'` "=== $i ==="
  make $i > make_$i.log 2>&1 || {
    cat make_$i.log
    echo "XXXX $i XXXX"
    break;
  }
done
