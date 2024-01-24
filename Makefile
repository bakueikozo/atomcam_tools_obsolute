# Makefile
.SILENT:

DOCKER_IMAGE=${shell sed -ne 's/^.*image:[ \t]*//p' docker-compose.yml}

build:
	-docker pull ${DOCKER_IMAGE} | awk '{ print } /Downloaded newer image/ { system("docker-compose down"); }'
	docker-compose ls | grep atomcam_tools > /dev/null || docker-compose up -d
	docker-compose exec builder /src/buildscripts/build_all | tee rebuild_`date +"%Y%m%d_%H%M%S"`.log

build-local:
	docker-compose ls | grep atomcam_tools > /dev/null || docker-compose up -d
	docker-compose exec builder /src/buildscripts/build_all | tee rebuild_`date +"%Y%m%d_%H%M%S"`.log

docker-build:
	# build container
	docker build -t ${DOCKER_IMAGE} . | tee docker-build_`date +"%Y%m%d_%H%M%S"`.log

login:
	docker-compose ls | grep atomcam_tools > /dev/null || docker-compose up -d
	docker-compose exec builder bash

lima:
	[ `uname -s` = "Darwin" ] && limactl start lima-docker.yml
