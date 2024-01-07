# Makefile
.SILENT:

build:
	-docker pull atomtools/atomtools:latest | awk '{ print } /Downloaded newer image/ { system("docker-compose down"); }'
	docker-compose ls | grep atomcam_tools > /dev/null || docker-compose up -d
	docker-compose exec builder /src/buildscripts/build_all | tee rebuild_`date +"%Y%m%d_%H%M%S"`.log

build-local:
	docker-compose ls | grep atomcam_tools > /dev/null || docker-compose up -d
	docker-compose exec builder /src/buildscripts/build_all | tee rebuild_`date +"%Y%m%d_%H%M%S"`.log

docker-build:
	# build container
	docker build -t atomtools/atomtools . | tee docker-build_`date +"%Y%m%d_%H%M%S"`.log

login:
	docker-compose ls | grep atomcam_tools > /dev/null || docker-compose up -d
	docker-compose exec builder bash

lima:
	[ `uname -s` = "Darwin" ] && limactl start lima-docker.yml
