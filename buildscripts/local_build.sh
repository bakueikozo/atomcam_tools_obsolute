#!/bin/sh

# tinyalsa
cd $BUILD_DIR
git clone https://github.com/tinyalsa/tinyalsa.git -b v2.0.0 --depth 1
cd tinyalsa
patch -p1 -N < /src/patches/tinyalsa/tinyalsa.patch
export CROSS_COMPILE="/atomtools/build/mips-gcc472-glibc216-64bit/bin/mips-linux-uclibc-gnu-"
export DESTDIR=/atomtools/build/mips-gcc472-glibc216-64bit/mips-linux-gnu/libc/uclibc
export PREFIX=/usr
export CFLAGS="-std=gnu99 -msoft-float"
make install
mkdir -p $TARGET_DIR/lib/modules
cp -dpf ${DESTDIR}/usr/lib/libtinyalsa.so.* $TARGET_DIR/lib/modules

# build libcallback.so
mkdir -p /atomtools/build/buildroot-2016.02/output/local
cp -pr /src/libcallback /atomtools/build/buildroot-2016.02/output/local
cd /atomtools/build/buildroot-2016.02/output/local/libcallback
make
[ $? != 0 ] && exit 1
mkdir -p $TARGET_DIR/lib/modules/
cp -dpf libcallback.so $TARGET_DIR/lib/modules/libcallback.so

# build webpage
mkdir -p /atomtools/build/buildroot-2016.02/output/web
cp -pr /src/web/webpack.config.js /src/web/package* /src/web/source /atomtools/build/buildroot-2016.02/output/web
cd /atomtools/build/buildroot-2016.02/output/web
rm -rf frontend
npm install -g npm@latest
npm install
./node_modules/.bin/webpack --mode production --progress
[ $? != 0 ] && exit 1
rm -rf $TARGET_DIR/var/www/bundle*
cp -pr frontend/* $TARGET_DIR/var/www
