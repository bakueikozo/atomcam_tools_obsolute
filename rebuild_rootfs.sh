#!/bin/sh

set -x

rm -rf openmiko/atomcam_configs
cp -pr atomcam_configs openmiko
cd openmiko
cp -pr atomcam_configs/overlay_initramfs/* initramfs_skeleton
cp -pr atomcam_configs/docker-compose.yml .

if ! docker ps | grep openmiko_builder_1 ; then
  docker-compose up -d
fi
docker-compose exec builder /src/atomcam_configs/scripts/rebuild_rootfs

cp uImage.lzma ../factory_t31_ZMC6tiIDQN
cp rootfs.ext2 ../rootfs_hack.ext2
cd ..
zip -ry atomcam_tools.zip factory_t31_ZMC6tiIDQN rootfs_hack.ext2 hostname authorized_keys
