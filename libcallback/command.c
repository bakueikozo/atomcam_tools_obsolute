#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

static const unsigned short commandPort = 4000;

int video_enable = 0;
int audio_enable = 0;
int jpeg_capture = 0;
pthread_cond_t jpeg_capture_cond;
pthread_mutex_t jpeg_capture_mutex;

static char *token_ptr;

struct commandTableSt {
  const char *cmd;
  char * (*func)(void);
};

char *VideoCapture() {

  char *p = strtok_r(NULL, " \t\r\n", &token_ptr);
  if(!p) return "error";
  if(!strcmp(p, "on")) {
    video_enable = 1;
    fprintf(stderr, "[command] video capute on\n", p);
    return "ok";
  }
  if(!strcmp(p, "off")) {
    video_enable = 0;
    fprintf(stderr, "[command] video capute off\n", p);
    return "ok";
  }
  return "error";
}

char *AudioCapture() {

  char *p = strtok_r(NULL, " \t\r\n", &token_ptr);
  if(!p) return "error";
  if(!strcmp(p, "on")) {
    audio_enable = 1;
    fprintf(stderr, "[command] audio capute on\n", p);
    return "ok";
  }
  if(!strcmp(p, "off")) {
    audio_enable = 0;
    fprintf(stderr, "[command] audio capute off\n", p);
    return "ok";
  }
  return "error";
}

char *JpegCapture() {

  pthread_mutex_lock(&jpeg_capture_mutex);
  jpeg_capture = 1;
  struct timespec tm;
  clock_gettime(CLOCK_REALTIME, &tm);
  tm.tv_nsec += 500 * 1000 * 1000;
  if(tm.tv_nsec >= 1000 * 1000 * 1000) {
    tm.tv_sec++;
    tm.tv_nsec -= 1000 * 1000 * 1000;
  }
  int res = pthread_cond_timedwait(&jpeg_capture_cond, &jpeg_capture_mutex, &tm);
  jpeg_capture = 0;
  pthread_mutex_unlock(&jpeg_capture_mutex);
  if(!res) return "ok";
  fprintf(stderr, "jpeg_capture error\n");
  return "error";
}

struct commandTableSt commandTable[] = {
  { "video", &VideoCapture },
  { "audio", &AudioCapture },
  { "jpeg", &JpegCapture },
};

static void *command_thread(void *arg) {

  struct sockaddr_in srcAddr;
  memset(&srcAddr, 0, sizeof(srcAddr));
  srcAddr.sin_port = htons(commandPort);
  srcAddr.sin_family = AF_INET;
  srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  int srcSocket = socket(AF_INET, SOCK_STREAM, 0);
  bind(srcSocket, (struct sockaddr *) &srcAddr, sizeof(srcAddr));
  listen(srcSocket, 1);
  while(1) {
    struct sockaddr_in dstAddr;
    int dstAddrSize = sizeof(dstAddr);
    int dstSocket = accept(srcSocket, (struct sockaddr *) &dstAddr, &dstAddrSize);
    if(strcmp(inet_ntoa(dstAddr.sin_addr), "127.0.0.1")) {
      fprintf(stderr, "Rejected request from %s\n", inet_ntoa(dstAddr.sin_addr));
      close(dstSocket);
      continue;
    }
    while(1) {
      char buf[256];
      int size = recv(dstSocket, buf, 255, 0);
      if(size <= 0) {
        close(dstSocket);
        break;
      }
      buf[size] = 0;
      char *p = strtok_r(buf, " \t\r\n", &token_ptr);
      if(!p) continue;
      int executed = 0;
      for(int i = 0; i < sizeof(commandTable) / sizeof(struct commandTableSt); i++) {
        if(!strcmp(p, commandTable[i].cmd)) {
          char *res = (*commandTable[i].func)();
          send(dstSocket, res, strlen(res) + 1, 0);
          executed = 1;
          break;
        }
      }
      if(!executed) fprintf(stderr, "command error : %s\n", p);
    }
  }
}

static void __attribute ((constructor)) command_init(void) {

  pthread_t thread;
  pthread_mutex_init(&jpeg_capture_mutex, NULL);
  pthread_cond_init(&jpeg_capture_cond, NULL);
  pthread_create(&thread, NULL, command_thread, NULL);
}
