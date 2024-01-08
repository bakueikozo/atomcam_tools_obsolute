#!/bin/bash
set -e

cd output/images
echo "atomcam" > hostname
touch authorized_keys
cp -dpf uImage.lzma factory_t31_ZMC6tiIDQN
mv rootfs.squashfs rootfs_hack.squashfs
rm -f /src/atomcam_tools.zip
zip -ry /src/atomcam_tools.zip factory_t31_ZMC6tiIDQN rootfs_hack.squashfs hostname authorized_keys
cp -f factory_t31_ZMC6tiIDQN rootfs_hack.squashfs /src/target
