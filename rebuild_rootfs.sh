#!/bin/sh

set -x

if ! docker ps | grep atomcam_tools_builder_1 ; then
  docker-compose up -d
fi
docker-compose exec builder /src/buildscripts/rebuild_rootfs
zip -ry ../atomcam_tools.zip factory_t31_ZMC6tiIDQN rootfs_hack.ext2 hostname authorized_keys
