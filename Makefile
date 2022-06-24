# Makefile
.SILENT:

build:
	-docker pull atomtools/atomtools:latest | grep 'Downloaded newer image' && docker-compose down
	docker-compose ls | grep atomcam_tools > /dev/null || docker-compose up -d
	docker-compose exec builder /src/buildscripts/build_all | tee rebuild_`date +"%Y%m%d_%H%M%S"`.log

docker-build:
	# build container
	docker build -t atomtools/atomtools . | tee docker-build_`date +"%Y%m%d_%H%M%S"`.log

login:
	-docker pull atomtools/atomtools:latest | grep 'Downloaded newer image' && docker-compose down > /dev/null
	docker-compose ls | grep atomcam_tools > /dev/null || docker-compose up -d
	docker-compose exec builder bash
