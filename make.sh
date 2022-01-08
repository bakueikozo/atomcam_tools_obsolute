#!/bin/sh

set -x

if docker ps | grep atomcam_tools_builder_1 ; then
  docker stop atomcam_tools_builder_1
fi
docker-compose up -d
docker-compose exec builder /src/buildscripts/build_all new
cd target
zip -ry ../atomcam_tools.zip factory_t31_ZMC6tiIDQN rootfs_hack.ext2 hostname authorized_keys
