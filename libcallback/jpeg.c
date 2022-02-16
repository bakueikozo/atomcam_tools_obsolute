#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern void CommandResponse(int fd, const char *res);
extern int local_sdk_video_get_jpeg_data(uint ch, unsigned char *buf, int size);

static const int JpegAllocateSize = 128 * 1024;
static const char *HttpResHeader = "Cache-Control: no-cache\nContent-Type: image/jpeg\n\n";

static pthread_mutex_t JpegDataMutex = PTHREAD_MUTEX_INITIALIZER;
static int JpegCaptureFd = 0;
static unsigned char *JpegBuffer[2] = { NULL, NULL };
static int JpegBufferSize[2] = { 0, 0 };
static int JpegBufferPtr = 0;

void JpegCaptureResponse(int fd) {

  int ptr = (JpegBufferSize[JpegBufferPtr] <= 0) ? JpegBufferPtr ^ 1 : JpegBufferPtr;
  if(JpegBufferSize[ptr] >= 0) {
    write(fd, HttpResHeader, strlen(HttpResHeader));
    write(fd, JpegBuffer[ptr], JpegBufferSize[ptr]);
    CommandResponse(fd, "");
  } else {
    CommandResponse(fd, "error : buffer size error");
  }
}

static void *JpegCaptureThread() {

  while(1) {
    pthread_mutex_lock(&JpegDataMutex);
    JpegBufferSize[JpegBufferPtr] = local_sdk_video_get_jpeg_data(0, JpegBuffer[JpegBufferPtr], JpegAllocateSize);
    if(JpegBufferSize[JpegBufferPtr] >= 0) JpegBufferPtr ^= 1;
    JpegCaptureResponse(JpegCaptureFd);
    JpegCaptureFd = 0;
  }
}

char *JpegCapture(int fd, char *tokenPtr) {

  if(JpegCaptureFd) {
    fprintf(stderr, "[command] jpeg capture error %d %d\n", JpegCaptureFd, fd);
    JpegCaptureResponse(fd);
    return NULL;
  }
  JpegCaptureFd = fd;
  pthread_mutex_unlock(&JpegDataMutex);
  return NULL;
}

static void __attribute ((constructor)) JpegInit(void) {

  if(!JpegBuffer[0]) JpegBuffer[0] = (unsigned char *)malloc(JpegAllocateSize);
  if(!JpegBuffer[1]) JpegBuffer[1] = (unsigned char *)malloc(JpegAllocateSize);
  if(!JpegBuffer[0] || !JpegBuffer[1]) {
    fprintf(stderr, "JpegInit malloc error\n");
    return;
  }
  pthread_mutex_lock(&JpegDataMutex);
  pthread_t thread;
  if(pthread_create(&thread, NULL, JpegCaptureThread, &JpegCaptureFd)) {
    fprintf(stderr, "pthread_create error\n");
    pthread_mutex_unlock(&JpegDataMutex);
    return;
  }
}
