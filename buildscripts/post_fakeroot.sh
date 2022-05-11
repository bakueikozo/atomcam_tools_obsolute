#!/bin/bash
set -e

echo "Executing pre filesystem image creation script"

# The environment variables BR2_CONFIG, HOST_DIR, STAGING_DIR,
# TARGET_DIR, BUILD_DIR, BINARIES_DIR and BASE_DIR are defined

/src/buildscripts/local_build.sh

find $TARGET_DIR -name .DS_Store -delete
cp /src/configs/atomhack.ver $TARGET_DIR/etc

DEFAULT_IMAGE_DIR="/atomtools/build/buildroot-2016.02/output/images"
BASE_DIR=${BASE_DIR:-/atomtools/build/buildroot-2016.02/output}
IMAGES="${BASE_DIR}/images"
HOST_DIR=${HOST_DIR:-/atomtools/build/buildroot-2016.02/output/host}
TARGET_DIR=${TARGET_DIR:-/atomtools/build/buildroot-2016.02/output/target}
