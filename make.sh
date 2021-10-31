#!/bin/sh

set -x

if docker ps | grep openmiko_builder_1 ; then
  docker stop openmiko_builder_1
fi

git clone https://github.com/openmiko/openmiko.git -b v0.0.28
cp -pr atomcam_configs openmiko
cd openmiko
cp -pr atomcam_configs/overlay_initramfs/* initramfs_skeleton

docker-compose up -d
docker-compose exec builder /src/atomcam_configs/scripts/build_all

mv uImage.lzma ../factory_t31_ZMC6tiIDQN
mv rootfs.ext2 ../rootfs_hack.ext2
cd ..
zip -ry atomcam_tools.zip factory_t31_ZMC6tiIDQN rootfs_hack.ext2 hostname authorized_keys
