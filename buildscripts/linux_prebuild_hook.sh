#!/bin/bash

# Create the cpio root filesystem that is embedded in the kernel
# This is a minimal root filesystem to bootstrap the bigger rootfs
/src/buildscripts/make_initramfs.sh /atomtools/build/buildroot-2016.02/output

# package patch
for i in `ls /src/patches/package` ; do
  cp -pr /src/patches/package/$i/*patch /atomtools/build/buildroot-2016.02/package/$i
done
