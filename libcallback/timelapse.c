#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

struct FrameCtrlSt {
  unsigned char *buf;
  int size;
  int nextIndex;
  struct timeval tm;
  int stat;
  int frameIndex;
  unsigned int dts; // us
  unsigned int pts; // us
  unsigned int reserve;
};

struct Mp4writeVideoConfigSt {
  int fps; // 20
  int width; // 1920
  int height; // 1080
  int bufSize; // 0x50000
  int flag; // 1
  void *callback; // callback func
};

struct Mp4WriteAudioConfigSt {
  int samplingRate; // 8000
  int format; // 16
  int channels; // 1
  int mode; // 2
};

extern int video_get_frame(int ch, int lockId, int mode, unsigned char *buf, struct FrameCtrlSt *frameCtrl);
extern int mp4write_request_handler(struct Mp4writeVideoConfigSt *vconfig, struct Mp4WriteAudioConfigSt *aconfig);
extern int mp4write_start_handler(int handler, char *file, struct Mp4writeVideoConfigSt *vconfig);
extern int mp4write_video_frame(int handler, struct FrameCtrlSt *frameCtrl);
extern int hevc2mp4_videostream(int handler, unsigned int *videoStreamCtrl);

extern int mp4write_stop_handler(int handler);
extern int mp4write_release_handler(int handler);
extern int IMP_ISP_Tuning_GetSensorFPS(int *fps_num, int *fps_den);

extern void CommandResponse(int fd, const char *res);

static int TimelapseFd = -1;
static int Busy = 0;
static int NumOfTimes = 0;
static int Interval = 0;
static int Count = 0;
static char File[256];
static char TmpFile[256];
static char ResBuf[256];
static pthread_mutex_t TimelapseMutex = PTHREAD_MUTEX_INITIALIZER;
static const int VideoBufSize = 0x50000;

// timelapse <file> <interval> <count>
// timelapse stop
char *Timelapse(int fd, char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(p && !strcmp(p, "stop")) {
    if(!Busy) return "error";
    Busy = 0;
    TimelapseFd = fd;
    return NULL;
  }

  if(!p) {
    if(!Busy) return "not operating.";
    snprintf(ResBuf, 255, "file: %s\ninterval: %dsec, count: %d/%d\n", File, Interval, Count, NumOfTimes);
    TimelapseFd = -1;
    return ResBuf;
  }
  if(Busy) return "error :　Already in operation.";

  strncpy(File, p, 255);
  strncpy(TmpFile, p, 255);
  strncat(TmpFile, ".tmp", 255);
  FILE *fp = fopen(TmpFile, "w");
  if(!fp) return strerror(errno);
  fclose(fp);

  p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  Interval = atoi(p);
  if(Interval < 1) Interval = 1;

  p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  NumOfTimes = atoi(p);
  if(NumOfTimes < 1) NumOfTimes = 1;
  Count = 0;

  Busy = 1;
  TimelapseFd = fd;
  pthread_mutex_unlock(&TimelapseMutex);
  return NULL;
}

static int writeTaskBusy = 0;

static void mp4writeCallback() {
  writeTaskBusy = 0;
}

static void *TimelapseThread() {

  while(1) {
    pthread_mutex_lock(&TimelapseMutex);
    struct FrameCtrlSt frameCtrl;
    memset(&frameCtrl, 0, sizeof(frameCtrl));

    // mp4 initialize
    int fps_num, fps_den;
    if(IMP_ISP_Tuning_GetSensorFPS(&fps_num, &fps_den)) {
      Busy = 0;
      CommandResponse(TimelapseFd, "error : can't get frame rate");
      continue;
    }
    struct Mp4writeVideoConfigSt videoConfig = {
      .fps = fps_num / fps_den,
      .width = 1920,
      .height = 1080,
      .bufSize = VideoBufSize,
      .flag = 0,
      .callback = &mp4writeCallback,
    };
    struct Mp4WriteAudioConfigSt audioConfig = {
      .samplingRate = 8000,
      .format = 16,
      .channels = 1,
      .mode = 2,
    };
    int mp4writeHandler = mp4write_request_handler(&videoConfig, &audioConfig);
    if(mp4writeHandler < 0) {
      Busy = 0;
      CommandResponse(TimelapseFd, "error : can't get mp4 handler");
      continue;
    }

    if(mp4write_start_handler(mp4writeHandler, TmpFile, &videoConfig)) {
      Busy = 0;
      CommandResponse(TimelapseFd, "error : can't start mp4 handler");
      continue;
    }

    frameCtrl.buf = malloc(VideoBufSize);
    if(!frameCtrl.buf) {
      Busy = 0;
      CommandResponse(TimelapseFd, "error : not enough memory to video buffer");
      continue;
    }

    CommandResponse(TimelapseFd, "ok");
    TimelapseFd = -1;

    writeTaskBusy = 1;
    int idx = 100;
    unsigned int dts = 1000 * 1000;
    struct timeval startTime;
    int firstFlag = 1;
    while(Busy) {
      int ret = video_get_frame(0, 0, 2, frameCtrl.buf, &frameCtrl);
      if(firstFlag) {
        gettimeofday(&startTime, NULL);
        firstFlag = 0;
      }
      frameCtrl.frameIndex = idx + Count;
      frameCtrl.nextIndex = idx + Count + 1;
      frameCtrl.dts = dts + (unsigned long long)Count * 1000 * 1000 / videoConfig.fps;
      frameCtrl.pts = frameCtrl.dts;

      fprintf(stderr, "[timelapse] %d/%d idx:%d timestamp:%d.%06d dts:%d\n", Count + 1, NumOfTimes, frameCtrl.frameIndex, frameCtrl.tm.tv_sec, frameCtrl.tm.tv_usec, frameCtrl.dts);

      if(!ret && !frameCtrl.stat) {
        if(mp4write_video_frame(mp4writeHandler, &frameCtrl)) {
          fprintf(stderr, "[timelapse] mp4write_video_frame error\n");
        } else {
          FILE *fp = fopen("/proc/sys/vm/drop_caches", "w");
          if(fp) {
            fwrite("1", 2, 1, fp);
            fclose(fp);
          }
          Count++;
          if(Count >= NumOfTimes) Busy = 0;
          fprintf(stdout, "[webhook] time_lapse_event %s %d/%d\n", File, Count, NumOfTimes);
        }
      }
      if(Busy) {
        struct timeval now;
        gettimeofday(&now, NULL);
        int us = (startTime.tv_sec + Interval * Count - now.tv_sec) * 1000 * 1000 + ((int)startTime.tv_usec - (int)now.tv_usec);
        if(us < 0) us = 0;
        usleep(us);
      }
    }
    if(!mp4write_stop_handler(mp4writeHandler)) {
      for(int i = 0; i < 100; i++) {
        if(!writeTaskBusy) break;
        usleep(50000);
      }
      mp4write_release_handler(mp4writeHandler);

      usleep(500000);
      rename(TmpFile, File);
      FILE *fp = fopen("/proc/sys/vm/drop_caches", "w");
      if(fp) {
        fwrite("1", 2, 1, fp);
        fclose(fp);
      }
      fprintf(stdout, "[webhook] time_lapse_finish %s %d/%d\n", File, Count, NumOfTimes);
      fprintf(stderr, "[timelapse] finish %s %d/%d\n", File, Count, NumOfTimes);
    }
    free(frameCtrl.buf);
    if(TimelapseFd >= 0) CommandResponse(TimelapseFd, "ok");
  }
}

static void __attribute ((constructor)) TimelapseInit(void) {

  if(getppid() != 1) return;
  pthread_mutex_lock(&TimelapseMutex);
  pthread_t thread;
  if(pthread_create(&thread, NULL, TimelapseThread, NULL)) {
    fprintf(stderr, "pthread_create error\n");
    pthread_mutex_unlock(&TimelapseMutex);
    return;
  }
}
