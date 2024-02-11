#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>

// contrast of image effect.
extern int IMP_ISP_Tuning_SetContrast(unsigned char contrast);
extern int IMP_ISP_Tuning_GetContrast(unsigned char *pcontrast);

// brightness of image effect.
extern int IMP_ISP_Tuning_SetBrightness(unsigned char bright);
extern int IMP_ISP_Tuning_GetBrightness(unsigned char *pbright);

// saturation of image effect.
extern int IMP_ISP_Tuning_SetSaturation(unsigned char sat);
extern int IMP_ISP_Tuning_GetSaturation(unsigned char *psat);

// sharpness of image effect.
extern int IMP_ISP_Tuning_SetSharpness(unsigned char sharpness);
extern int IMP_ISP_Tuning_GetSharpness(unsigned char *psharpness);

// AE compensation.
// AE compensation parameters can adjust the target of the image AE.
extern int IMP_ISP_Tuning_SetAeComp(int comp);
extern int IMP_ISP_Tuning_GetAeComp(int *comp);

// AE Max parameters.
extern int IMP_ISP_Tuning_SetAe_IT_MAX(unsigned int it_max);
extern int IMP_ISP_Tuning_GetAE_IT_MAX(unsigned int *it_max);

// DPC Strength.
extern int IMP_ISP_Tuning_SetDPC_Strength(unsigned int ratio);
extern int IMP_ISP_Tuning_GetDPC_Strength(unsigned int *ratio);

// DRC Strength.
extern int IMP_ISP_Tuning_SetDRC_Strength(unsigned int ratio);
extern int IMP_ISP_Tuning_GetDRC_Strength(unsigned int *ratio);

// highlight intensity controls.
extern int IMP_ISP_Tuning_SetHiLightDepress(uint32_t strength);
extern int IMP_ISP_Tuning_GetHiLightDepress(uint32_t *strength);

// Set 3D noise reduction intensity.
extern int IMP_ISP_Tuning_SetTemperStrength(uint32_t ratio);

// Set 2D noise reduction intensity.
extern int IMP_ISP_Tuning_SetSinterStrength(uint32_t ratio);

// Max value of sensor analog gain.
extern int IMP_ISP_Tuning_SetMaxAgain(uint32_t gain);
extern int IMP_ISP_Tuning_GetMaxAgain(uint32_t *gain);

// Max value of sensor Digital gain.
extern int IMP_ISP_Tuning_SetMaxDgain(uint32_t gain);
extern int IMP_ISP_Tuning_GetMaxDgain(uint32_t *gain);

// ISP image mirror(horizontal) effect function (enable/disable)
extern int IMP_ISP_Tuning_SetISPHflip(int mode);
extern int IMP_ISP_Tuning_GetISPHflip(int *pmode);

// ISP image mirror(vertical) effect function (enable/disable)
extern int IMP_ISP_Tuning_SetISPVflip(int mode);
extern int IMP_ISP_Tuning_GetISPVflip(int *pmode);

extern char *VideoCapture(int fd, char *p, char *tokenPtr);

static char *Flip(char *tokenPtr);
static char *Contrast(char *tokenPtr);
static char *Brightness(char *tokenPtr);
static char *Saturation(char *tokenPtr);
static char *Sharpness(char *tokenPtr);
static char *Sinter(char *tokenPtr);
static char *Temper(char *tokenPtr);
static char *AEComp(char *tokenPtr);
static char *AEItMax(char *tokenPtr);
static char *DPC(char *tokenPtr);
static char *DRC(char *tokenPtr);
static char *HiLight(char *tokenPtr);
static char *AGain(char *tokenPtr);
static char *DGain(char *tokenPtr);
static char *Bitrate(char *tokenPtr);

struct CommandTableSt {
  const char *cmd;
  char * (*func)(char *);
};
static struct CommandTableSt VideoCommandTable[] = {
  { "flip",      &Flip }, // flip [normal/flip/mirror/flip_mirror]
  { "cont",      &Contrast }, // cont 0 - 255(center:128)
  { "bri",       &Brightness }, // bri 0 - 255(center:128)
  { "sat",       &Saturation }, // sat 0 - 255(center:128)
  { "sharp",     &Sharpness }, // sharp 0 - 255(center:128)
  { "sinter",    &Sinter }, // sinter 0 - 255(center:128)
  { "temper",    &Temper }, // temper 0 - 255(center:128)
  { "aecomp",    &AEComp }, // aecomp 0 - 255
  { "aeitmax",   &AEItMax }, // aeitmax 0-
  { "dpc",       &DPC }, // dpc 0 - 255
  { "drc",       &DRC }, // drc 0 - 255
  { "hilight",   &HiLight }, // hilight 0 - 10
  { "again",     &AGain }, // again 0 -
  { "dgain",     &DGain }, // dgain 0 -
  { "bitrate",   &Bitrate }, // bitrate <ch> 10-3000(kbps)|auto
};

static int (*real_local_sdk_video_set_kbps)(int ch, int kbps);
static int userBitrate[4] = { 0, 0, 0, 0 };
static int appBitrate[4] = { 960, 180, 0, 800 };
static char videoResBuf[256];

static void __attribute ((constructor)) video_control_init(void) {

  real_local_sdk_video_set_kbps = dlsym(dlopen ("/system/lib/liblocalsdk.so", RTLD_LAZY), "local_sdk_video_set_kbps");
}

char *VideoCommand(int fd, char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(p) {
    for(int i = 0; i < sizeof(VideoCommandTable) / sizeof(struct CommandTableSt); i++) {
      if(!strcmp(p, VideoCommandTable[i].cmd)) return (*VideoCommandTable[i].func)(tokenPtr);
    }
  }
  return VideoCapture(fd, p, tokenPtr);
}

int local_sdk_video_set_kbps(int ch, int kbps) {

  if((ch == 0) || (ch == 1) || (ch == 3)) {
    appBitrate[ch] = kbps;
    if(userBitrate[ch]) {
      kbps = userBitrate[ch];
      fprintf(stderr, "video_set_kbps ch%d: %d -> %d\n", ch, appBitrate[ch], kbps);
    } else {
      fprintf(stderr, "video_set_kbps ch%d: %d\n", ch, kbps);
    }
  }
  return real_local_sdk_video_set_kbps(ch, kbps);
}

static char *Flip(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    int vflip, hflip;
    IMP_ISP_Tuning_GetISPHflip(&hflip);
    IMP_ISP_Tuning_GetISPVflip(&vflip);
    if(!hflip && !vflip) return "normal";
    if(hflip && !vflip) return "flip";
    if(!hflip && vflip) return "mirror";
    return "flip_mirror";
  }

  if(!strcmp(p, "normal")) {
    IMP_ISP_Tuning_SetISPVflip(0);
    IMP_ISP_Tuning_SetISPHflip(0);
  } else if(!strcmp(p, "flip")) {
    IMP_ISP_Tuning_SetISPVflip(1);
    IMP_ISP_Tuning_SetISPHflip(0);
  } else if(!strcmp(p, "mirror")) {
    IMP_ISP_Tuning_SetISPVflip(0);
    IMP_ISP_Tuning_SetISPHflip(1);
  } else if(!strcmp(p, "flip_mirror")) {
    IMP_ISP_Tuning_SetISPVflip(1);
    IMP_ISP_Tuning_SetISPHflip(1);
  } else {
    return "error";
  }
  return "ok";
}

static char *Contrast(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned char cont;
    IMP_ISP_Tuning_GetContrast(&cont);
    sprintf(videoResBuf, "%d\n", cont);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetContrast(atoi(p));
  return res ? "error": "ok";
}

static char *Brightness(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned char bri;
    IMP_ISP_Tuning_GetBrightness(&bri);
    sprintf(videoResBuf, "%d\n", bri);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetBrightness(atoi(p));
  return res ? "error": "ok";
}

static char *Saturation(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned char sat;
    IMP_ISP_Tuning_GetSaturation(&sat);
    sprintf(videoResBuf, "%d\n", sat);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetSaturation(atoi(p));
  return res ? "error": "ok";
}

static char *Sharpness(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned char sharpness;
    IMP_ISP_Tuning_GetSharpness(&sharpness);
    sprintf(videoResBuf, "%d\n", sharpness);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetSharpness(atoi(p));
  return res ? "error": "ok";
}

static char *AEComp(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    int comp;
    IMP_ISP_Tuning_GetAeComp(&comp);
    sprintf(videoResBuf, "%d\n", comp);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetAeComp(atoi(p));
  return res ? "error": "ok";
}

static char *AEItMax(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int itMax;
    IMP_ISP_Tuning_GetAE_IT_MAX(&itMax);
    sprintf(videoResBuf, "%d\n", itMax);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetAe_IT_MAX(atoi(p));
  return res ? "error": "ok";
}

static char *Sinter(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  int res = IMP_ISP_Tuning_SetSinterStrength(atoi(p));
  return res ? "error": "ok";
}

static char *Temper(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  int res = IMP_ISP_Tuning_SetTemperStrength(atoi(p));
  return res ? "error": "ok";
}

static char *DPC(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int dpc;
    IMP_ISP_Tuning_GetDPC_Strength(&dpc);
    sprintf(videoResBuf, "%d\n", dpc);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetDPC_Strength(atoi(p));
  return res ? "error": "ok";
}

static char *DRC(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int drc;
    IMP_ISP_Tuning_GetDRC_Strength(&drc);
    sprintf(videoResBuf, "%d\n", drc);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetDRC_Strength(atoi(p));
  return res ? "error": "ok";
}

static char *HiLight(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int strength;
    IMP_ISP_Tuning_GetHiLightDepress(&strength);
    sprintf(videoResBuf, "%d\n", strength);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetHiLightDepress(atoi(p));
  return res ? "error": "ok";
}

static char *AGain(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int gain;
    IMP_ISP_Tuning_GetMaxAgain(&gain);
    sprintf(videoResBuf, "%d\n", gain);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetMaxAgain(atoi(p));
  return res ? "error": "ok";
}

static char *DGain(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    unsigned int gain;
    IMP_ISP_Tuning_GetMaxDgain(&gain);
    sprintf(videoResBuf, "%d\n", gain);
    return videoResBuf;
  }
  int res = IMP_ISP_Tuning_SetMaxDgain(atoi(p));
  return res ? "error": "ok";
}

static char *Bitrate(char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  int ch = atoi(p);
  if((ch != 0) && (ch != 1) && (ch != 3)) return "error";
  p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    if(userBitrate[ch]) {
      sprintf(videoResBuf, "%d\n", userBitrate[ch]);
    } else {
      sprintf(videoResBuf, "auto %d\n", appBitrate[ch]);
    }
    return videoResBuf;
  }
  if(!strcmp(p, "auto")) {
    userBitrate[ch] = 0;
    fprintf(stderr, "video_set_kbps ch%d: %d\n", ch, appBitrate[ch]);
    real_local_sdk_video_set_kbps(ch, appBitrate[ch]);
  } else {
    int kbps = atoi(p);
    if((kbps < 10) || (kbps > 3000)) return "error";
    userBitrate[ch] = kbps;
    fprintf(stderr, "video_set_kbps ch%d: %d\n", ch, userBitrate[ch]);
    real_local_sdk_video_set_kbps(ch, userBitrate[ch]);
  }
  return "ok";
}
