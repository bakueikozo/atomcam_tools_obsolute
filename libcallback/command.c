#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

extern int local_sdk_motor_get_position(float *step,float *angle);
extern int local_sdk_motor_move_abs_angle(float pan, float tilt, int speed, void (*done)(float a, float b), void (*canceled)(void), int mode);

static const unsigned short CommandPort = 4000;

int VideoCaptureEnable = 0;
int AudioCaptureEnable = 0;
int JpegCaptureTrigger = 0;

static int SelfPipe[2];
static char *TokenPtr;

void CommandResponse(int fd, const char *res) {

  unsigned char buf[256];
  buf[0] = strlen(res) + 1;
  buf[1] = fd;
  strncpy((char *)buf + 2, res, 253);
  write(SelfPipe[1], &buf, buf[0] + 2);
}

static char *MotorGetPos(int fd) {

  float pan; // 0-355
  float tilt; // 0-180
  int ret = local_sdk_motor_get_position(&pan, &tilt);
  static char motorResBuf[256];
  sprintf(motorResBuf, "MotorPos : %f %f\nok", pan, tilt);
  return motorResBuf;
}


static int motorFd = 0;
static void motor_move_done(float a, float b) {
  if(motorFd) CommandResponse(motorFd, "ok");
  motorFd = 0;
}

static void motor_move_canceled() {
  if(motorFd) CommandResponse(motorFd, "error");
  motorFd = 0;
}

static char *MotorSetPos(int fd) {

  char *p = strtok_r(NULL, " \t\r\n", &TokenPtr);
  if(!p) return "error";
  float pan = atof(p); // 0-355
  if((pan < 0.0) || (pan > 355.0)) return "error";

  p = strtok_r(NULL, " \t\r\n", &TokenPtr);
  if(!p) return "error";
  float tilt = atof(p); // 0-180
  if((tilt < 0.0) || (tilt > 180.0)) return "error";

  if(motorFd) return "error";
  motorFd = fd;

  int speed = 9;
  int pri = 2; // 0: high - 3: low
  int res = local_sdk_motor_move_abs_angle(pan, tilt, speed, &motor_move_done, &motor_move_canceled, pri);
  return NULL;
}

static char *VideoCapture(int fd) {

  char *p = strtok_r(NULL, " \t\r\n", &TokenPtr);
  if(!p) return "error";
  if(!strcmp(p, "on")) {
    VideoCaptureEnable = 1;
    fprintf(stderr, "[command] video capute on\n", p);
    return "ok";
  }
  if(!strcmp(p, "off")) {
    VideoCaptureEnable = 0;
    fprintf(stderr, "[command] video capute off\n", p);
    return "ok";
  }
  return "error";
}

static char *AudioCapture(int fd) {

  char *p = strtok_r(NULL, " \t\r\n", &TokenPtr);
  if(!p) return "error";
  if(!strcmp(p, "on")) {
    AudioCaptureEnable = 1;
    fprintf(stderr, "[command] audio capute on\n", p);
    return "ok";
  }
  if(!strcmp(p, "off")) {
    AudioCaptureEnable = 0;
    fprintf(stderr, "[command] audio capute off\n", p);
    return "ok";
  }
  return "error";
}

static char *JpegCapture(int fd) {

  if(JpegCaptureTrigger) {
    fprintf(stderr, "[command] jpeg error %d\n", JpegCaptureTrigger);
    CommandResponse(JpegCaptureTrigger, "error");
  }
  JpegCaptureTrigger = fd;
  return NULL;
}

struct CommandTableSt {
  const char *cmd;
  char * (*func)(int);
};

struct CommandTableSt CommandTable[] = {
  { "video",  &VideoCapture },
  { "audio",  &AudioCapture },
  { "jpeg",   &JpegCapture },
  { "getpos", &MotorGetPos },
  { "setpos", &MotorSetPos },
};

static void *CommandThread(void *arg) {

  static const int MaxConnect = 255;
  int maxFd = 0;
  fd_set targetFd;

  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if(listenSocket < 0) {
    fprintf(stderr, "socket : %s\n", strerror(errno));
    return NULL;
  }
  int sock_optval = 1;
  if(setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR,
                &sock_optval, sizeof(sock_optval)) == -1) {
    fprintf(stderr, "setsockopt : %s\n", strerror(errno));
    close(listenSocket);
    return NULL;
  }

  struct sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(CommandPort);
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(listenSocket, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
    fprintf(stderr, "bind : %s\n", strerror(errno));
    close(listenSocket);
    return NULL;
  }

  if(listen(listenSocket, MaxConnect) == -1) {
    fprintf(stderr, "listen : %s\n", strerror(errno));
    close(listenSocket);
    return NULL;
  }

  FD_ZERO(&targetFd);
  FD_SET(listenSocket, &targetFd);
  maxFd = listenSocket;
  FD_SET(SelfPipe[0], &targetFd);
  maxFd = (SelfPipe[0] > maxFd) ? SelfPipe[0] : maxFd;
  if(maxFd >= MaxConnect) maxFd = MaxConnect - 1;

  while(1) {
    fd_set checkFDs;
    memcpy(&checkFDs, &targetFd, sizeof(targetFd));
    if(select(maxFd + 1, &checkFDs, NULL, NULL, NULL) == -1) {
      fprintf(stderr, "select error : %s\n", strerror(errno));
    } else {
      for(int fd = maxFd; fd >= 0; fd--) {
        if(FD_ISSET(fd, &checkFDs)) {
          if(fd == SelfPipe[0]) {
            while(1) {
              unsigned char buf[256];
              int length = read(SelfPipe[0], buf, 2);
              if(length <= 1) break;
              int resSize = buf[0];
              int resFd = buf[1];
              length = read(SelfPipe[0], buf, resSize);
              if(length < resSize) break;
              char *res = (char *)buf;
              strcat(res, "\n");
              send(resFd, res, strlen(res) + 1, 0);
              close(resFd);
              FD_CLR(resFd, &targetFd);
            }
          } else if(fd == listenSocket) {
            struct sockaddr_in dstAddr;
            int len = sizeof(dstAddr);
            int newSocket = accept(fd, (struct sockaddr *)&dstAddr, (socklen_t *)&len);
            if(newSocket < 0) {
              fprintf(stderr, "Socket::Accept Error\n");
              continue;
            }
            if(strcmp(inet_ntoa(dstAddr.sin_addr), "127.0.0.1")) {
              fprintf(stderr, "Rejected request from %s\n", inet_ntoa(dstAddr.sin_addr));
              close(newSocket);
              continue;
            }
            int flag = fcntl(newSocket, F_GETFL, 0);
            fcntl(newSocket, F_SETFL, O_NONBLOCK|flag);
            FD_SET(newSocket, &targetFd);
            maxFd = (newSocket > maxFd) ? newSocket : maxFd;
            if(maxFd >= MaxConnect) maxFd = MaxConnect - 1;
          } else {
            char buf[256];
            int size = recv(fd, buf, 255, 0);
            if(!size) {
              FD_CLR(fd, &targetFd);
              break;
            }
            if(size < 0) {
              close(fd);
              FD_CLR(fd, &targetFd);
              break;
            }
            buf[size] = 0;
            char *p = strtok_r(buf, " \t\r\n", &TokenPtr);
            if(!p) continue;
            int executed = 0;
            for(int i = 0; i < sizeof(CommandTable) / sizeof(struct CommandTableSt); i++) {
              if(!strcmp(p, CommandTable[i].cmd)) {
                char *res = (*CommandTable[i].func)(fd);
                if(res) {
                  send(fd, res, strlen(res) + 1, 0);
                  char cr = '\n';
                  send(fd, &cr, 1, 0);
                  close(fd);
                  FD_CLR(fd, &targetFd);
                }
                executed = 1;
                break;
              }
            }
            if(!executed) {
              char *res = "error";
              send(fd, res, strlen(res) + 1, 0);
              close(fd);
              FD_CLR(fd, &targetFd);
              fprintf(stderr, "command error : %s\n", p);
            }
          }
         }
      }
    }
  }
}

static void __attribute ((constructor)) command_init(void) {

  if(pipe(SelfPipe)) {
    fprintf(stderr, "pipe error\n");
    return;
  }
  int flag = fcntl(SelfPipe[0], F_GETFL, 0);
  fcntl(SelfPipe[0], F_SETFL, O_NONBLOCK|flag);
  flag = fcntl(SelfPipe[1], F_GETFL, 0);
  fcntl(SelfPipe[1], F_SETFL, O_NONBLOCK|flag);

  pthread_t thread;
  pthread_create(&thread, NULL, CommandThread, NULL);
}
