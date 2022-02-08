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

static const unsigned short commandPort = 4000;

int video_enable = 0;
int audio_enable = 0;
int jpeg_capture = 0;
static int SelfPipe[2];

static char *token_ptr;

struct commandTableSt {
  const char *cmd;
  char * (*func)(int);
};

void commandResponse(int fd, const char *res) {

  unsigned char buf[256];
  buf[0] = fd;
  strcpy((char *)buf + 1, res);
  write(SelfPipe[1], &buf, strlen(res) + 2);
}

char *VideoCapture(int fd) {

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

char *AudioCapture(int fd) {

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

char *JpegCapture(int fd) {

  if(jpeg_capture) commandResponse(jpeg_capture, "error");
  jpeg_capture = fd;
  return NULL;
}

struct commandTableSt commandTable[] = {
  { "video", &VideoCapture },
  { "audio", &AudioCapture },
  { "jpeg", &JpegCapture },
};

static void *command_thread(void *arg) {

  static const int MaxConnect = 255;
  int MaxFd = 0;
  int PortTable[MaxConnect];
  fd_set TargetFd;

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
  saddr.sin_port = htons(commandPort);
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

  FD_ZERO(&TargetFd);
  for(int i = 0; i < MaxConnect; i++) {
    PortTable[i] = 0;
  }

  PortTable[listenSocket] = 0;
  FD_SET(listenSocket, &TargetFd);
  MaxFd = listenSocket;
  FD_SET(SelfPipe[0], &TargetFd);
  MaxFd = (SelfPipe[0] > MaxFd) ? SelfPipe[0] : MaxFd;
  if(MaxFd >= MaxConnect) MaxFd = MaxConnect - 1;

  while(1) {
    fd_set checkFDs;
    memcpy(&checkFDs, &TargetFd, sizeof(TargetFd));
    if(select(MaxFd + 1, &checkFDs, NULL, NULL, NULL) == -1) {
      fprintf(stderr, "select error : %s\n", strerror(errno));
    } else {
      for(int fd = MaxFd; fd >= 0; fd--) {
        if(FD_ISSET(fd, &checkFDs)) {
          if(fd == SelfPipe[0]) {
            unsigned char buf[256];
            read(SelfPipe[0], buf, 256);
            int resFd = buf[0];
            char *res = (char *)buf + 1;
            strcat(res, "\n");
            send(resFd, res, strlen(res) + 1, 0);
            close(resFd);
            FD_CLR(resFd, &TargetFd);
            PortTable[resFd] = 0;
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
            PortTable[newSocket] = 1;
            FD_SET(newSocket, &TargetFd);
            MaxFd = (newSocket > MaxFd) ? newSocket : MaxFd;
            if(MaxFd >= MaxConnect) MaxFd = MaxConnect - 1;
          } else if(PortTable[fd] > 0) {
            char buf[256];
            int size = recv(fd, buf, 255, 0);
            if(!size) {
              FD_CLR(fd, &TargetFd);
              break;
            }
            if(size < 0) {
              close(fd);
              FD_CLR(fd, &TargetFd);
              PortTable[fd] = 0;
              break;
            }
            buf[size] = 0;
            char *p = strtok_r(buf, " \t\r\n", &token_ptr);
            if(!p) continue;
            int executed = 0;
            for(int i = 0; i < sizeof(commandTable) / sizeof(struct commandTableSt); i++) {
              if(!strcmp(p, commandTable[i].cmd)) {
                char *res = (*commandTable[i].func)(fd);
                if(res) {
                  send(fd, res, strlen(res) + 1, 0);
                  close(fd);
                  FD_CLR(fd, &TargetFd);
                  PortTable[fd] = 0;
                }
                executed = 1;
                break;
              }
            }
            if(!executed) {
              char *res = "error";
              send(fd, res, strlen(res) + 1, 0);
              close(fd);
              FD_CLR(fd, &TargetFd);
              PortTable[fd] = 0;
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
  pthread_create(&thread, NULL, command_thread, NULL);
}
