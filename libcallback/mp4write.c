#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

/* ===== This file is currently not in use ===== */

static int (*original_mp4write_start_handler)(void *handler, char *file, void *config);

int mp4write_start_handler(void *handler, char *file, void *config) {

  if(!strncmp(file, "/tmp/", 5)) {
    char buf[64];
    strncpy(buf, file + 5, 30);
    strcpy(file, "/media/mmc/tmp/");
    strcat(file, buf);
  }
  return (original_mp4write_start_handler)(handler, file, config);
}

static void __attribute ((constructor)) mp4write_init(void) {

  original_mp4write_start_handler = dlsym(dlopen("/system/lib/libmp4rw.so", RTLD_LAZY), "mp4write_start_handler");
}
