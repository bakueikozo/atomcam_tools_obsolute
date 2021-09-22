#!/bin/sh

set -x
#sudo docker cp b95f6c00696a:/openmiko/build/buildroot-2016.02/output/images/rootfs.ext2 ./rootfs_hack.ext2
rm libcallback.so
mips-linux-uclibc-gnu-gcc -fPIC -shared -o libcallback.so filesnooper.c -ldl
cp libcallback.so modules
cd workspace
chmod a+x Test/test.sh
tar cfv ../Test.tar Test factory
cd ..

rm atomcam_tools.zip
zip -ry atomcam_tools.zip swap modules scripts Test.tar rootfs_hack.ext2


