#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <tinyalsa/pcm.h>

struct frames_st {
  void *buf;
  size_t length;
};
typedef int (* framecb)(struct frames_st *);

static uint32_t (*real_local_sdk_audio_set_pcm_frame_callback)(int ch, void *callback);
static void *audio_pcm_cb = NULL;

struct audio_capture_st {
  struct pcm *pcm;
  int enable;
};
struct audio_capture_st audio_capture[] = {
  {
    .pcm = NULL,
    .enable = 0,
  },
  {
    .pcm = NULL,
    .enable = 0,
  },
};

char *AudioCapture(int fd, char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  int ch = 0;
  if(p && (!strcmp(p, "0") || !strcmp(p, "1"))) {
    ch = atoi(p);
    p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  }
  if(!p) {
    if(!audio_capture[ch].pcm) return "disabled";
    return audio_capture[ch].enable ? "on" : "off";
  }
  if(!strcmp(p, "on")) {
    audio_capture[ch].enable = 1;
    printf("[command] audio %d capute on\n", ch);
    return "ok";
  }
  if(!strcmp(p, "off")) {
    audio_capture[ch].enable = 0;
    printf("[command] audio %d capute off\n", ch);
    return "ok";
  }
  return "error";
}

static uint32_t audio_pcm_capture(struct frames_st *frames) {

  const struct pcm_config config = {
    .channels = 1,
    .rate = 8000,
    .format = PCM_FORMAT_S16_LE,
    .period_size = 320,
    .period_count = 16,
    .start_threshold = 0,
    .silence_threshold = 0,
    .silence_size = 0,
    .stop_threshold = 0,
  };

  for(int ch = 0; ch < 2; ch++) {
    if(!audio_capture[ch].pcm) {
      audio_capture[ch].pcm = pcm_open(0, ch, PCM_OUT | PCM_MMAP, &config);
      if(audio_capture[ch].pcm == NULL) {
          fprintf(stderr, "failed to allocate memory for PCM%d\n", ch);
      } else if(!pcm_is_ready(audio_capture[ch].pcm)) {
        fprintf(stderr, "failed to open PCM%d : %s\n", ch, pcm_get_error(audio_capture[ch].pcm));
        pcm_close(audio_capture[ch].pcm);
        audio_capture[ch].pcm = NULL;
      }
    }

    if(audio_capture[ch].pcm && audio_capture[ch].enable) {
      if(pcm_mmap_avail(audio_capture[ch].pcm) >= config.period_size * config.period_count / 4) {
        int err = pcm_writei(audio_capture[ch].pcm, frames->buf, pcm_bytes_to_frames(audio_capture[ch].pcm, frames->length));
        if(err < 0) fprintf(stderr, "pcm_writei ch%d err=%d\n", ch, err);
      } else {
        fprintf(stderr, "[audio] drop packet: ch%d %d\n", ch, frames->length);
      }
    }
  }
  return ((framecb)audio_pcm_cb)(frames);
}

uint32_t local_sdk_audio_set_pcm_frame_callback(int ch, void *callback) {

  fprintf(stderr, "local_sdk_audio_set_pcm_frame_callback streamChId=%d, callback=0x%x\n", ch, callback);
  if(ch == 0) {
    audio_pcm_cb = callback;
    fprintf(stderr,"enc func injection save audio_pcm_cb=0x%x\n", audio_pcm_cb);
    callback = audio_pcm_capture;
  }
  return real_local_sdk_audio_set_pcm_frame_callback(ch, callback);
}

static void __attribute ((constructor)) audio_callback_init(void) {

  real_local_sdk_audio_set_pcm_frame_callback = dlsym(dlopen("/system/lib/liblocalsdk.so", RTLD_LAZY), "local_sdk_audio_set_pcm_frame_callback");
}
