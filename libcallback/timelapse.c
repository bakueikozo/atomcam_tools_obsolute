/*
  usage:
   timelapse <file> <interval> <count>   : start timelapse record
   timelapse mp4 <file>                  : reconvert to mp4
   timelapse close                       : close timelapse record (completely closed)
   timelapse stop                        : stop timelapse record (possible to restart)
   timelapse restart                     : restart timelapse record
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

static const char *TimelapseInfoFile = "/media/mmc/time_lapse.info";

struct TimeLapseInfoSt {
  int interval;
  int count;
  int numOfTimes;
  unsigned int mdatSize;
  unsigned int spsOffset;
  unsigned int spsSize;
  unsigned int ppsOffset;
  unsigned int ppsSize;
  char mp4file[256];
  char mp4xfile[256];
  char stszfile[256];
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

typedef enum {
  State_Ready,
  State_Recording,
  State_ConvertToMP4,
} StateSt;

typedef enum {
  Directive_Nop,
  Directive_Start,
  Directive_Restart,
  Directive_ToMP4,
  Directive_Stop,
  Directive_Close,
} DirectiveSt;

static const unsigned char mp4Header[] = {
  0x00, 0x00, 0x00, 0x20, 0x66, 0x74, 0x79, 0x70, // ftyp
  0x69, 0x73, 0x6F, 0x6D, 0x00, 0x00, 0x02, 0x00, // isom
  0x69, 0x73, 0x6F, 0x6D, 0x69, 0x73, 0x6F, 0x32, // isom,iso2,avc1,mp41
  0x61, 0x76, 0x63, 0x31, 0x6D, 0x70, 0x34, 0x31,
  0x00, 0x00, 0x00, 0x08, 0x66, 0x72, 0x65, 0x65, // free
  0x00, 0x00, 0x00, 0x00, 0x6D, 0x64, 0x61, 0x74, // mdat
};

static const unsigned char moov[] = {
  0xFF, 0xFF, 0xFF, 0xFF, 0x6d, 0x6f, 0x6f, 0x76, // 0x000 moov   >>> offset 0x00: uint32 moovSize = 0x74 + trakSize
  0x00, 0x00, 0x00, 0x6C, 0x6D, 0x76, 0x68, 0x64, //       mvhd
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x010
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, //       timescale 0x03e8(1000)
  0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0x00, // 0x020 duration >>> offset 0x20: uint32 duration = frameCount / 20fps * timescale
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, // 0x030
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, // 0x040
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, // 0x050
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x060
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, // 0x070 >>> offset 0x74: uint32 trakSize = 0x64 + mdiaSize
  0x74, 0x72, 0x61, 0x6B, 0x00, 0x00, 0x00, 0x5C, //       trak
  0x74, 0x6B, 0x68, 0x64, 0x00, 0x00, 0x00, 0x03, // 0x080 tkhd
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, // 0x090
  0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, //       duration >>> offset 0x98: uint32 duration = frameCount / 20fps * timescale
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x0a0
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x0b0
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x0c0
  0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
  0x07, 0x80, 0x00, 0x00, 0x04, 0x38, 0x00, 0x00, // 0x0d0 width 0x0780(1920, height 0x0438(1080)
  0xFF, 0xFF, 0xFF, 0xFF, 0x6D, 0x64, 0x69, 0x61, //       mdia >>> offset 0xd8: uint32 mdiaSize = 0x50 + minfSize
  0x00, 0x00, 0x00, 0x20, 0x6D, 0x64, 0x68, 0x64, // 0x0e0 mdhd
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe8, // 0x0f0 timescale 0x000003e8(1000)
  0xFF, 0xFF, 0xFF, 0xFF, 0x55, 0xC4, 0x00, 0x00, //       duration >>> offset 0xf8: uint32 duration = frameCount / 20fps * timescale

  0x00, 0x00, 0x00, 0x28, 0x68, 0x64, 0x6C, 0x72, // 0x100 hdlr
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x76, 0x69, 0x64, 0x65, 0x00, 0x00, 0x00, 0x00, // 0x110 vide
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x56, 0x69, 0x64, 0x65, 0x6F, 0x48, 0x64, 0x00, // 0x120 VideoHd
  0xFF, 0xFF, 0xFF, 0xFF, 0x6D, 0x69, 0x6E, 0x66, //       minf >>> offset 0x128: uint32 minfSize = 0x08 + stblSize
  0x00, 0x00, 0x00, 0x14, 0x76, 0x6D, 0x68, 0x64, // 0x130 vmhd
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, // 0x140
  0x64, 0x69, 0x6E, 0x66, 0x00, 0x00, 0x00, 0x1C, //       dinf
  0x64, 0x72, 0x65, 0x66, 0x00, 0x00, 0x00, 0x00, // 0x150 dref
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0C,
  0x75, 0x72, 0x6C, 0x20, 0x00, 0x00, 0x00, 0x01, // 0x160 url
  0xFF, 0xFF, 0xFF, 0xFF, 0x73, 0x74, 0x62, 0x6C, //       stbl >>> offset 0x168: uint32 stblSize = 0x08 + sttsSize(0x18) + stsdSize(0xa1) + stscSize + stszSize + stcoSize
  0x00, 0x00, 0x00, 0x18, 0x73, 0x74, 0x74, 0x73, // 0x170 stts
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //
  0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x32, // 0x180 >>> offset 0x180: uint32 sampleCount(frameCount), sample_delta
  0xFF, 0xFF, 0xFF, 0xFF, 0x73, 0x74, 0x73, 0x64, //       stsd >>> offset 0x188: uint32 stsdSize = 0x10 + avc1Size(0x91)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // 0x190
  0xFF, 0xFF, 0xFF, 0xFF, 0x61, 0x76, 0x63, 0x31, //       avc1 >>> offset 0x198: uint32 avc1Size = 0x56 + avcCSize(0x3b)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // 0x1a0
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x1b0
  0x07, 0x80, 0x04, 0x38, 0x00, 0x48, 0x00, 0x00, //       width 0x0780(1920) height 0x0438(1080) hRes 0x00480000(72.0)
  0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x1c0 vRes 0x00480000(72.0)
  0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x1d0
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x1e0
  0x00, 0x00, 0x00, 0x18, 0xff, 0xff, 0xFF, 0xFF, //        >>> offset 0x1ee: uint32 avcCSize = 16 + SPSSize(0x24) + 3 + PPSSize(4)
  0xFF, 0xFF, 0x61, 0x76, 0x63, 0x43, 0x01, 0x4D, // 0x1f0  avcC
  0x00, 0x29, 0xff, 0xe1,
  //       >>> offset 0x1f4: uint16 SPSSize
  //       SPS(SPSSize)
  //       >>> offset 0x1f6 + SPSSize: uint8 PPSCount(1)
  //       >>> offset 0x1f7 + SPSSize: uint16 PPSSize
  //       PPS(PPSSize)
  //
  //       >>> offset 0x1f9 + SPSSize + PPSSize: uint32 stscSize
  //       stsc entry: frameCount % 8 ? 2 : 1  sample / chunk = 8
  //       >>> offset 0x1f9 + SPSSize + PPSSize + stscSize: uint32 stszSize
  //       stsz entry:frameCount
  //       >>> offset 0x1f9 + SPSSize + PPSSize + stscSize + stszSize: uint32 stcoSize
  //       stco entry:frameCount
};

extern int video_get_frame(int ch, int lockId, int mode, unsigned char *buf, struct FrameCtrlSt *frameCtrl);
extern void CommandResponse(int fd, const char *res);

static struct TimeLapseInfoSt TimeLapseInfo;
static char ResBuf[256];
static pthread_mutex_t TimelapseMutex = PTHREAD_MUTEX_INITIALIZER;
static const int VideoBufSize = 0x50000;
static int TimelapseFd = -1;
static DirectiveSt Directive = Directive_Nop;
static StateSt State = State_Ready;

char *Timelapse(int fd, char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    if(TimeLapseInfo.count >= TimeLapseInfo.numOfTimes) return "not operating.";
    snprintf(ResBuf, 255, "file: %s\ninterval: %dsec, count: %d/%d\n", TimeLapseInfo.mp4file, TimeLapseInfo.interval, TimeLapseInfo.count, TimeLapseInfo.numOfTimes);
    return ResBuf;
  }

  if(!strcmp(p, "stop")) {
    if(State != State_Recording) return "error";
    Directive = Directive_Stop;
    TimelapseFd = fd;
    return NULL;
  }

  if(!strcmp(p, "close")) {
    if(State != State_Recording) return "error";
    Directive = Directive_Close;
    TimelapseFd = fd;
    return NULL;
  }

  if(State != State_Ready) return "error :　Already in operation.";

  if(!strcmp(p, "restart")) {
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
    Directive = Directive_Restart;
    TimelapseFd = fd;
    pthread_mutex_unlock(&TimelapseMutex);
    return NULL;
  }

  int mp4Flag = 0;
  if(p && !strcmp(p, "mp4")) {
    mp4Flag = 1;
    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  }

  strncpy(TimeLapseInfo.mp4file, p, 250);
  char *q = strrchr(TimeLapseInfo.mp4file, '.');
  if(!q) q = TimeLapseInfo.mp4file + strlen(TimeLapseInfo.mp4file);
  strcpy(q, ".mp4");

  strncpy(TimeLapseInfo.mp4xfile, TimeLapseInfo.mp4file, 250);
  q = strrchr(TimeLapseInfo.mp4xfile, '.');
  strcpy(q, "._mp4");

  strncpy(TimeLapseInfo.stszfile, TimeLapseInfo.mp4file, 250);
  q = strrchr(TimeLapseInfo.stszfile, '.');
  strcpy(q, ".stsz");

  if(!mp4Flag) {
    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
    if(!p) return "error";
    TimeLapseInfo.interval = atoi(p);
    if(TimeLapseInfo.interval < 1) TimeLapseInfo.interval = 1;

    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
    if(!p) return "error";
    TimeLapseInfo.numOfTimes = atoi(p);
    if(TimeLapseInfo.numOfTimes < 1) TimeLapseInfo.numOfTimes = 1;

    TimeLapseInfo.count = 0;
    TimeLapseInfo.mdatSize = 8;
    TimeLapseInfo.spsSize = 0;
    TimeLapseInfo.ppsSize = 0;

    FILE *fp = fopen(TimeLapseInfo.mp4xfile, "w");
    if(!fp) return strerror(errno);
    fwrite(mp4Header, sizeof(mp4Header), 1, fp);
    fclose(fp);

    fp = fopen(TimeLapseInfo.stszfile, "w");
    if(!fp) return strerror(errno);
    fclose(fp);
  }

  Directive = mp4Flag ? Directive_ToMP4 : Directive_Start;
  TimelapseFd = fd;
  pthread_mutex_unlock(&TimelapseMutex);
  return NULL;
}

static int writeTaskBusy = 0;
static void mp4writeCallback() {
  writeTaskBusy = 0;
}

static char *AppendMoov() {

  unsigned int fps = 20;
  unsigned int timeScale = 1000;

  if(!TimeLapseInfo.count || !TimeLapseInfo.spsSize || !TimeLapseInfo.ppsSize) return "error : no timelapse data";

  FILE *fp = fopen(TimeLapseInfo.mp4xfile, "r+"); // Do not open in append mode when seeking and writing.
  if(!fp) return "error : can't open mp4 file";

  FILE *fp2 = fopen(TimeLapseInfo.stszfile, "r");
  if(!fp2) return "error : can't open stsz file";

  unsigned char *buf = malloc(sizeof(moov) + 2 + TimeLapseInfo.spsSize + 3 + TimeLapseInfo.ppsSize);
  if(!buf) return "error : can't allocate moov memory";

  memcpy(buf, moov, sizeof(moov));
  fseek(fp, TimeLapseInfo.spsOffset, SEEK_SET);
  fread(buf + sizeof(moov) + 2, TimeLapseInfo.spsSize, 1, fp);
  buf[sizeof(moov)] = TimeLapseInfo.spsSize >> 8;
  buf[sizeof(moov)  + 1] = TimeLapseInfo.spsSize;
  fseek(fp, TimeLapseInfo.ppsOffset, SEEK_SET);
  fread(buf + sizeof(moov) + 2 + TimeLapseInfo.spsSize + 3, TimeLapseInfo.ppsSize, 1, fp);
  buf[sizeof(moov) + 2 + TimeLapseInfo.spsSize] = 1;
  buf[sizeof(moov) + 2 + TimeLapseInfo.spsSize + 1] = TimeLapseInfo.ppsSize >> 8;
  buf[sizeof(moov) + 2 + TimeLapseInfo.spsSize + 2] = TimeLapseInfo.ppsSize;

  unsigned int stcoChunkCount = (TimeLapseInfo.count + 7) / 8;
  unsigned int stcoSize = 16 + stcoChunkCount * 4;
  unsigned int stscEntry = (stcoChunkCount > 1) && (TimeLapseInfo.count % 8) ? 2 : 1;
  unsigned int stscSize = 16 + stscEntry * 12;
  unsigned int stszSize = 20 + TimeLapseInfo.count * 4;

  unsigned int avcCSize = 16 + TimeLapseInfo.spsSize + 3 + TimeLapseInfo.ppsSize;
  buf[0x1ee] = avcCSize >> 24;
  buf[0x1ef] = avcCSize >> 16;
  buf[0x1f0] = avcCSize >> 8;
  buf[0x1f1] = avcCSize;

  unsigned int avc1Size = 0x56 + avcCSize;
  buf[0x198] = avc1Size >> 24;
  buf[0x199] = avc1Size >> 16;
  buf[0x19a] = avc1Size >> 8;
  buf[0x19b] = avc1Size;

  unsigned int stsdSize = 0x10 + avc1Size;
  buf[0x188] = stsdSize >> 24;
  buf[0x189] = stsdSize >> 16;
  buf[0x18a] = stsdSize >> 8;
  buf[0x18b] = stsdSize;

  unsigned int sttsSize = 0x18;

  unsigned int stblSize = 0x08 + sttsSize + stsdSize + stscSize + stszSize + stcoSize;
  buf[0x168] = stblSize >> 24;
  buf[0x169] = stblSize >> 16;
  buf[0x16a] = stblSize >> 8;
  buf[0x16b] = stblSize;

  unsigned int vmhdSize = 20;
  unsigned int dinfSize = 36;

  unsigned int minfSize = 0x08 + vmhdSize + dinfSize + stblSize;
  buf[0x128] = minfSize >> 24;
  buf[0x129] = minfSize >> 16;
  buf[0x12a] = minfSize >> 8;
  buf[0x12b] = minfSize;

  unsigned int mdiaSize = 0x50 + minfSize;
  buf[0x0d8] = mdiaSize >> 24;
  buf[0x0d9] = mdiaSize >> 16;
  buf[0x0da] = mdiaSize >> 8;
  buf[0x0db] = mdiaSize;

  unsigned int trakSize = 0x64 + mdiaSize;
  buf[0x074] = trakSize >> 24;
  buf[0x075] = trakSize >> 16;
  buf[0x076] = trakSize >> 8;
  buf[0x077] = trakSize;

  unsigned int moovSize = 0x74 + trakSize;
  buf[0x000] = moovSize >> 24;
  buf[0x001] = moovSize >> 16;
  buf[0x002] = moovSize >> 8;
  buf[0x003] = moovSize;

  unsigned int duration = TimeLapseInfo.count * timeScale / fps;
  buf[0x01c] = timeScale >> 24;
  buf[0x01d] = timeScale >> 16;
  buf[0x01e] = timeScale >> 8;
  buf[0x01f] = timeScale;

  buf[0x020] = duration >> 24;
  buf[0x021] = duration >> 16;
  buf[0x022] = duration >> 8;
  buf[0x023] = duration;

  buf[0x098] = duration >> 24;
  buf[0x099] = duration >> 16;
  buf[0x09a] = duration >> 8;
  buf[0x09b] = duration;

  buf[0x0f4] = timeScale >> 24;
  buf[0x0f5] = timeScale >> 16;
  buf[0x0f6] = timeScale >> 8;
  buf[0x0f7] = timeScale;

  buf[0x0f8] = duration >> 24;
  buf[0x0f9] = duration >> 16;
  buf[0x0fa] = duration >> 8;
  buf[0x0fb] = duration;

  buf[0x180] = TimeLapseInfo.count >> 24;
  buf[0x181] = TimeLapseInfo.count >> 16;
  buf[0x182] = TimeLapseInfo.count >> 8;
  buf[0x183] = TimeLapseInfo.count;

  unsigned int sampleDelta = timeScale / fps;
  buf[0x184] = sampleDelta >> 24;
  buf[0x185] = sampleDelta >> 16;
  buf[0x186] = sampleDelta >> 8;
  buf[0x187] = sampleDelta;

  fseek(fp, TimeLapseInfo.mdatSize + sizeof(mp4Header) - 8, SEEK_SET);
  fwrite(buf, sizeof(moov) + 2 + TimeLapseInfo.spsSize + 3 + TimeLapseInfo.ppsSize, 1, fp);

  // stsc
  buf[0] = stscSize >> 24;
  buf[1] = stscSize >> 16;
  buf[2] = stscSize >> 8;
  buf[3] = stscSize;
  buf[4] = 's';
  buf[5] = 't';
  buf[6] = 's';
  buf[7] = 'c';
  memset(buf + 8, 0, (2 + 6) * 4);
  buf[15] = stscEntry;
  buf[19] = 1;
  buf[23] = TimeLapseInfo.count >= 8 ? 8 : TimeLapseInfo.count;
  buf[27] = 1;
  if(stscEntry > 1) {
    buf[31] = (TimeLapseInfo.count / 8) + 1;
    buf[35] = (TimeLapseInfo.count % 8);
    buf[39] = 1;
  }
  fwrite(buf, stscSize, 1, fp);

  // stsz
  buf[0] = stszSize >> 24;
  buf[1] = stszSize >> 16;
  buf[2] = stszSize >> 8;
  buf[3] = stszSize;
  buf[4] = 's';
  buf[5] = 't';
  buf[6] = 's';
  buf[7] = 'z';
  memset(buf + 8, 0, 8);
  buf[16] = TimeLapseInfo.count >> 24;
  buf[17] = TimeLapseInfo.count >> 16;
  buf[18] = TimeLapseInfo.count >> 8;
  buf[19] = TimeLapseInfo.count;
  fwrite(buf, 20, 1, fp);
  for(int i = 0; i < TimeLapseInfo.count; i++) {
    fread(buf, 8, 1, fp2);
    fwrite(buf + 4, 4, 1, fp);
  }

  // stco
  buf[0] = stcoSize >> 24;
  buf[1] = stcoSize >> 16;
  buf[2] = stcoSize >> 8;
  buf[3] = stcoSize;
  buf[4] = 's';
  buf[5] = 't';
  buf[6] = 'c';
  buf[7] = 'o';
  memset(buf + 8, 0, 4);
  buf[12] = stcoChunkCount >> 24;
  buf[13] = stcoChunkCount >> 16;
  buf[14] = stcoChunkCount >> 8;
  buf[15] = stcoChunkCount;
  fwrite(buf, 16, 1, fp);

  for(int i = 0; i < stcoChunkCount; i++) {
    fseek(fp2, 8 * i * 8, SEEK_SET);
    fread(buf, 4, 1, fp2);
    fwrite(buf, 4, 1, fp);
  }
  fclose(fp2);
  fclose(fp);

  rename(TimeLapseInfo.mp4xfile, TimeLapseInfo.mp4file);
  unlink(TimeLapseInfo.stszfile);
  fp = fopen("/proc/sys/vm/drop_caches", "w");
  if(fp) {
    fwrite("1", 2, 1, fp);
    fclose(fp);
  }
  return "ok";
}

static void *TimelapseThread() {

  while(1) {
    pthread_mutex_lock(&TimelapseMutex);

    struct FrameCtrlSt frameCtrl;
    memset(&frameCtrl, 0, sizeof(frameCtrl));
    frameCtrl.buf = malloc(VideoBufSize);
    if(!frameCtrl.buf) {
      CommandResponse(TimelapseFd, "error : not enough memory to video buffer");
      goto finalize;
    }

    State = State_Recording;
    struct timeval startTime = { .tv_sec = 0, .tv_usec = 0 };
    while((Directive < Directive_ToMP4)) {
      int ret = video_get_frame(0, 0, 2, frameCtrl.buf, &frameCtrl);
      if(ret) fprintf(stderr, "[timelapse] error video_get_frame %d\n", ret);
      if(frameCtrl.stat) fprintf(stderr, "[timelapse] error video_get_frame frame.sstat %d\n", frameCtrl.stat);
      int lastSC = -1;
      for(int i = 0; i < 0x100; i++) {
        if((frameCtrl.buf[i] == 0) && (frameCtrl.buf[i + 1] == 0) && (frameCtrl.buf[i + 2] == 0) && (frameCtrl.buf[i + 3] == 1)) {
          int nulUnitType = frameCtrl.buf[i + 4] & 0x1f;
          if(lastSC >= 0) {
            int size = i - lastSC - 4;
            frameCtrl.buf[lastSC] = size >> 24;
            frameCtrl.buf[lastSC + 1] = size >> 16;
            frameCtrl.buf[lastSC + 2] = size >> 8;
            frameCtrl.buf[lastSC + 3] = size;
            int lastNulType = frameCtrl.buf[lastSC + 4] & 0x1f;
            if(!TimeLapseInfo.spsSize && (lastNulType == 7)) { // SPS
              TimeLapseInfo.spsOffset = lastSC + sizeof(mp4Header) + 4;
              TimeLapseInfo.spsSize = size;
            }
            if(!TimeLapseInfo.ppsSize && (lastNulType == 8)) { // PPS
              TimeLapseInfo.ppsOffset = lastSC + sizeof(mp4Header) + 4;
              TimeLapseInfo.ppsSize = size;
            }
          }
          lastSC = i;
          if(nulUnitType == 5) {
            int size = frameCtrl.size - i - 4;
            frameCtrl.buf[i] = size >> 24;
            frameCtrl.buf[i + 1] = size >> 16;
            frameCtrl.buf[i + 2] = size >> 8;
            frameCtrl.buf[i + 3] = size;
            break;
          }
          i+= 4;
        }
      }

      if(!startTime.tv_sec) {
        gettimeofday(&startTime, NULL);
        if((Directive == Directive_Start) || (Directive == Directive_Restart)) {
          startTime.tv_sec -= TimeLapseInfo.interval * TimeLapseInfo.count;
          CommandResponse(TimelapseFd, "ok");
          TimelapseFd = -1;
          Directive = Directive_Nop;
        }
      }

      if(!ret && !frameCtrl.stat) {
        unsigned int mdatOffset = TimeLapseInfo.mdatSize + sizeof(mp4Header) - 8;
        FILE *fp = fopen(TimeLapseInfo.mp4xfile, "r+"); // Do not open in append mode when seeking and writing.
        if(fp) {
          unsigned char buf[4];
          fseek(fp, TimeLapseInfo.mdatSize + sizeof(mp4Header) - 8, SEEK_SET);
          fwrite(frameCtrl.buf, frameCtrl.size, 1, fp);
          TimeLapseInfo.mdatSize += frameCtrl.size; // sizeof(mp4Header) - 8 = mdat size offset
          fseek(fp, sizeof(mp4Header) - 8, SEEK_SET);
          buf[0] = TimeLapseInfo.mdatSize >> 24;
          buf[1] = TimeLapseInfo.mdatSize >> 16;
          buf[2] = TimeLapseInfo.mdatSize >> 8;
          buf[3] = TimeLapseInfo.mdatSize;
          fwrite(buf, 4, 1, fp);
          fclose(fp);
        }
        fp = fopen(TimeLapseInfo.stszfile, "a");
        if(fp) {
          unsigned char buf[8];
          buf[0] = mdatOffset >> 24;
          buf[1] = mdatOffset >> 16;
          buf[2] = mdatOffset >> 8;
          buf[3] = mdatOffset;
          buf[4] = frameCtrl.size >> 24;
          buf[5] = frameCtrl.size >> 16;
          buf[6] = frameCtrl.size >> 8;
          buf[7] = frameCtrl.size;
          fwrite(buf, 8, 1, fp);
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
      fprintf(stdout, "[webhook] time_lapse_event %s %d/%d\n", TimeLapseInfo.mp4xfile, TimeLapseInfo.count, TimeLapseInfo.numOfTimes);

      if(TimeLapseInfo.count >= TimeLapseInfo.numOfTimes) break;
      if(Directive >= Directive_ToMP4) break;

      while(1) {
        struct timeval now;
        gettimeofday(&now, NULL);
        int ms = (startTime.tv_sec + TimeLapseInfo.interval * TimeLapseInfo.count - now.tv_sec) * 1000  + ((int)startTime.tv_usec - (int)now.tv_usec) / 1000;
        if(ms <= 0) break;
        if(Directive >= Directive_ToMP4) break;
        if(ms > 1000) ms = 1000;
        usleep(ms * 1000);
      }
    }
    State = State_ConvertToMP4;

    if(Directive == Directive_Stop) {
      CommandResponse(TimelapseFd, "ok");
      goto finalize;
    }
    if(Directive == Directive_Close) {
      TimeLapseInfo.numOfTimes = TimeLapseInfo.count;
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
    }

    char *res = AppendMoov();
    if(!strcmp(res, "ok")) {
      fprintf(stdout, "[webhook] time_lapse_finish %s %d/%d\n", TimeLapseInfo.mp4file, TimeLapseInfo.count, TimeLapseInfo.numOfTimes);
      fprintf(stderr, "[timelapse] finish %s %d/%d\n", TimeLapseInfo.mp4file, TimeLapseInfo.count, TimeLapseInfo.numOfTimes);
    }
    CommandResponse(TimelapseFd, res);

finalize:
    free(frameCtrl.buf);
    TimelapseFd = -1;
    State = State_Ready;
    Directive = Directive_Nop;
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
