# Makefile

build:
	docker-compose ps | grep -v exited | grep builder || docker-compose up -d
	docker-compose exec builder /src/buildscripts/build_all | tee rebuild_`date +"%Y%m%d_%H%M%S"`.log

docker-build:
	# build container
	docker build -t atomtools/atomtools . | tee docker-build_`date +"%Y%m%d_%H%M%S"`.log

login:
	docker-compose ps | grep -v exited | grep builder || docker-compose up -d
	docker-compose exec builder bash
