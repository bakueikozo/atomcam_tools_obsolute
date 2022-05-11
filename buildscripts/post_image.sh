#!/bin/bash
set -e

cd output/images
echo "atomcam" > hostname
touch authorized_keys
cp -dpf uImage.lzma factory_t31_ZMC6tiIDQN
mv rootfs.ext2 rootfs_hack.ext2
rm -f /src/atomcam_tools.zip
zip -ry /src/atomcam_tools.zip factory_t31_ZMC6tiIDQN rootfs_hack.ext2 hostname authorized_keys
cp -f factory_t31_ZMC6tiIDQN rootfs_hack.ext2 /src/target
