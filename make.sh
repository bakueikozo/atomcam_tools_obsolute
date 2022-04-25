#!/bin/sh

if docker-compose ps | grep builder ; then
  docker-compose down --rmi all
fi
docker-compose up -d
docker-compose exec builder /src/buildscripts/build_all new
[ $? != 0 ] && exit $?
cd target
zip -ry ../atomcam_tools.zip factory_t31_ZMC6tiIDQN rootfs_hack.ext2 hostname authorized_keys
