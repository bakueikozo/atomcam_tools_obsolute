/*
  usage:
   timelapse <file> <interval> <count>   : start timelapse record
   timelapse mp4 <file>                  : reconvert to mp4
   timelapse stop                        : stop timelapse record
   timelapse                             : status or restart
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

static const char *TimelapseInfoFile = "/media/mmc/timelapse.info";

struct TimeLapseInfoSt {
  char mp4file[256];
  char h264file[256];
  int interval;
  int count;
  int numOfTimes;
};

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

static const unsigned char sei0[] = {
  0x00, 0x00, 0x00, 0x01, 0x26, 0x05, 0x00, 0x1b,
  0xaa, 0xdc, 0x45, 0xe9, 0xbd, 0xe6, 0xd9, 0x48,
  0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee,
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

static struct TimeLapseInfoSt TimeLapseInfo;
static char ResBuf[256];
static pthread_mutex_t TimelapseMutex = PTHREAD_MUTEX_INITIALIZER;
static const int VideoBufSize = 0x50000;
static int ConvToMP4 = 0;
static int TimelapseFd = -1;
static int Busy = 0;
static int Restart = 0;
static int Stop = 0;

char *Timelapse(int fd, char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(p && !strcmp(p, "stop")) {
    if(!Busy) return "error";
    Stop = 1;
    TimelapseFd = fd;
    return NULL;
  }

  ConvToMP4 = 0;
  if(p && !strcmp(p, "mp4")) {
    ConvToMP4 = 1;
    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  }

  if(!p) {
    if(!Busy) {
      FILE *fp = fopen(TimelapseInfoFile, "r");
      if(fp) {
        if(fread(&TimeLapseInfo, sizeof(TimeLapseInfo), 1, fp) != 1) {
          memset(&TimeLapseInfo, 0, sizeof(TimeLapseInfo));
        }
        fclose(fp);
      } else {
        memset(&TimeLapseInfo, 0, sizeof(TimeLapseInfo));
      }
      if(TimeLapseInfo.count >= TimeLapseInfo.numOfTimes) return "not operating.";
      Restart = 1;
      Busy = 1;
      TimelapseFd = fd;
      pthread_mutex_unlock(&TimelapseMutex);
      return NULL;
    }
    snprintf(ResBuf, 255, "file: %s\ninterval: %dsec, count: %d/%d\n", TimeLapseInfo.mp4file, TimeLapseInfo.interval, TimeLapseInfo.count, TimeLapseInfo.numOfTimes);
    TimelapseFd = -1;
    return ResBuf;
  }
  if(Busy) return "error :　Already in operation.";

  strncpy(TimeLapseInfo.mp4file, p, 250);
  char *q = strrchr(TimeLapseInfo.mp4file, '.');
  if(!q) q = TimeLapseInfo.mp4file + strlen(TimeLapseInfo.mp4file);
  strcpy(q, ".mp4");

  strncpy(TimeLapseInfo.h264file, TimeLapseInfo.mp4file, 250);
  q = strrchr(TimeLapseInfo.h264file, '.');
  strcpy(q, ".h264");

  if(!ConvToMP4) {
    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
    if(!p) return "error";
    TimeLapseInfo.interval = atoi(p);
    if(TimeLapseInfo.interval < 1) TimeLapseInfo.interval = 1;

    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
    if(!p) return "error";
    TimeLapseInfo.numOfTimes = atoi(p);
    if(TimeLapseInfo.numOfTimes < 1) TimeLapseInfo.numOfTimes = 1;

    FILE *fp = fopen(TimeLapseInfo.h264file, "w");
    if(!fp) return strerror(errno);
    fclose(fp);
  }

  FILE *fp = fopen(TimeLapseInfo.mp4file, "w");
  if(!fp) return strerror(errno);
  fclose(fp);

  TimeLapseInfo.count = 0;
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

    frameCtrl.buf = malloc(VideoBufSize);
    if(!frameCtrl.buf) {
      Busy = 0;
      CommandResponse(TimelapseFd, "error : not enough memory to video buffer");
      TimelapseFd = -1;
      continue;
    }

    if(!ConvToMP4) {
      if(Restart) {
        CommandResponse(TimelapseFd, "restart");
      } else {
        CommandResponse(TimelapseFd, "ok");
      }
      TimelapseFd = -1;
    }
    struct timeval startTime = { .tv_sec = 0, .tv_usec = 0 };
    while(!ConvToMP4) {
      int ret = video_get_frame(0, 0, 2, frameCtrl.buf, &frameCtrl);
      if(ret) fprintf(stderr, "[timelapse] error video_get_frame %d\n", ret);
      if(frameCtrl.stat) fprintf(stderr, "[timelapse] error video_get_frame frame.sstat %d\n", frameCtrl.stat);
      if(!startTime.tv_sec) {
        gettimeofday(&startTime, NULL);
        if(Restart) startTime.tv_sec -= TimeLapseInfo.interval * TimeLapseInfo.count;
        Restart = 0;
      }

      if(!ret && !frameCtrl.stat) {
        FILE *fp = fopen(TimeLapseInfo.h264file, "a+");
        if(fp) {
          unsigned char buf[64];
          int p = sizeof(sei0);
          memcpy(buf, sei0, p);
          buf[p++] = TimeLapseInfo.count;
          buf[p++] = 0xff;
          buf[p++] = frameCtrl.size >> 16;
          buf[p++] = frameCtrl.size >> 8;
          buf[p++] = frameCtrl.size;
          buf[p++] = 0xff;
          buf[p++] = 0;
          buf[p++] = 0x80;
          fwrite(buf, p, 1, fp);
          fwrite(frameCtrl.buf, frameCtrl.size, 1, fp);
          fclose(fp);
        }
      }
      TimeLapseInfo.count++;
      FILE *fp = fopen(TimelapseInfoFile, "w");
      if(fp) {
        fwrite(&TimeLapseInfo, sizeof(TimeLapseInfo), 1, fp);
        fclose(fp);
      }
      fp = fopen("/proc/sys/vm/drop_caches", "w");
      if(fp) {
        fwrite("1", 2, 1, fp);
        fclose(fp);
      }
      fprintf(stderr, "[timelapse] %d/%d\n", TimeLapseInfo.count, TimeLapseInfo.numOfTimes);
      fprintf(stdout, "[webhook] time_lapse_event %s %d/%d\n", TimeLapseInfo.mp4file, TimeLapseInfo.count, TimeLapseInfo.numOfTimes);

      if(TimeLapseInfo.count >= TimeLapseInfo.numOfTimes) break;
      if(Stop) break;

      struct timeval now;
      gettimeofday(&now, NULL);
      int us = (startTime.tv_sec + TimeLapseInfo.interval * TimeLapseInfo.count - now.tv_sec) * 1000 * 1000 + ((int)startTime.tv_usec - (int)now.tv_usec);
      if(us < 0) us = 0;
      usleep(us);
    }
    Stop = 0;

    struct timeval start;
    gettimeofday(&start, NULL);

    int fps_num;
    int fps_den;
    if(IMP_ISP_Tuning_GetSensorFPS(&fps_num, &fps_den)) {
      Busy = 0;
      CommandResponse(TimelapseFd, "error : can't get frame rate");
      TimelapseFd = -1;
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

    writeTaskBusy = 1;
    int mp4writeHandler = mp4write_request_handler(&videoConfig, &audioConfig);
    if(mp4writeHandler < 0) {
      Busy = 0;
      CommandResponse(TimelapseFd, "error : can't get mp4 handler");
      TimelapseFd = -1;
      continue;
    }

    if(mp4write_start_handler(mp4writeHandler, TimeLapseInfo.mp4file, &videoConfig)) {
      Busy = 0;
      CommandResponse(TimelapseFd, "error : can't start mp4 handler");
      TimelapseFd = -1;
      continue;
    }

    struct stat st;
    stat(TimeLapseInfo.h264file, &st);
    int time = st.st_ctime;
    FILE *fp = fopen(TimeLapseInfo.h264file, "r");
    unsigned char *buf = frameCtrl.buf;
    int idx = 100;
    unsigned int dts = 1000 * 1000;
    int count = 0;
    while(!feof(fp)) {
      unsigned char buf[64];
      int readSize = fread(buf, 24 + 8, 1, fp);
      if(readSize != 1) break;
      if(memcmp(buf, sei0, sizeof(sei0))) break;
      frameCtrl.size = (buf[26] << 16) | (buf[27] << 8) | buf[28];
      readSize = fread(frameCtrl.buf, frameCtrl.size, 1, fp);
      frameCtrl.frameIndex = idx + count;
      frameCtrl.nextIndex = idx + count + 1;
      frameCtrl.stat = 0;
      frameCtrl.dts = dts + (unsigned long long)count * 1000 * 1000 / videoConfig.fps;
      frameCtrl.pts = frameCtrl.dts;
      frameCtrl.tm.tv_sec = time + count;
      frameCtrl.tm.tv_usec = 0;
      frameCtrl.reserve = 0;
      // fprintf(stderr, "[timelapse] %d/%d idx:%d timestamp:%d.%06d dts:%d\n", count + 1, TimeLapseInfo.numOfTimes, frameCtrl.frameIndex, frameCtrl.tm.tv_sec, frameCtrl.tm.tv_usec, frameCtrl.dts);
      if(mp4write_video_frame(mp4writeHandler, &frameCtrl)) {
        fprintf(stderr, "[timelapse] mp4write_video_frame error\n");
      }
      count++;
    }
    fclose(fp);

    if(!mp4write_stop_handler(mp4writeHandler)) {
      for(int i = 0; i < 100; i++) {
        if(!writeTaskBusy) break;
        usleep(50000);
      }
      mp4write_release_handler(mp4writeHandler);

      usleep(500000);
      FILE *fp = fopen("/proc/sys/vm/drop_caches", "w");
      if(fp) {
        fwrite("1", 2, 1, fp);
        fclose(fp);
      }
      fprintf(stdout, "[webhook] time_lapse_finish %s %d/%d\n", TimeLapseInfo.mp4file, count, TimeLapseInfo.numOfTimes);
      fprintf(stderr, "[timelapse] finish %s %d/%d\n", TimeLapseInfo.mp4file, count, TimeLapseInfo.numOfTimes);
    }
    free(frameCtrl.buf);

    struct timeval finish;
    gettimeofday(&finish, NULL);
    fprintf(stderr, "start : %d.%06d  finish : %d.%06d\n", start.tv_sec, start.tv_usec, finish.tv_sec, finish.tv_usec);

    CommandResponse(TimelapseFd, "ok");
    Busy = 0;
    TimelapseFd = -1;
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
