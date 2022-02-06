# libcallback.so

CC = /openmiko/build/mips-gcc472-glibc216-64bit/bin/mips-linux-uclibc-gnu-gcc
CFLAGS = -fPIC -std=gnu99 -shared -ldl -ltinyalsa
CC_SRCS = video_callback.c audio_callback.c setlinebuf.c mmc_format.c curl.c freopen.c opendir.c remove.c command.c
TARGET = libcallback.so

all: ${TARGET}

${TARGET}: ${CC_SRCS}
	${CC} ${CFLAGS} -o ${TARGET} ${CC_SRCS}
