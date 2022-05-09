#!/bin/sh

if docker-compose ps | grep builder ; then
  docker-compose down --rmi all
fi
docker-compose up -d
LOGFILE=build_`date +"%Y%m%d_%H%M%S"`.log
docker-compose exec builder /src/buildscripts/build_all new | tee $LOGFILE
[ $? != 0 ] && exit 1
cd target
zip -ry ../atomcam_tools.zip factory_t31_ZMC6tiIDQN rootfs_hack.ext2 hostname authorized_keys
