#include <stdio.h>
#include <dlfcn.h>
#include <time.h>
#include <sys/time.h>

extern int motorFd;
extern int motorLastMovedTime;

static struct tm *(*original_gmtime_r)(const time_t *timep, struct tm *result);

static void __attribute ((constructor)) gmtime_r_hook_init(void) {

  original_gmtime_r = dlsym(dlopen ("/lib/libc.so.0", RTLD_LAZY), "gmtime_r");
}

struct tm *gmtime_r(const time_t *timep, struct tm *result) {

  original_gmtime_r(timep, result);
  // While the camera is moving, the AI process is disabled by returning a day of the week that does not exist when motion is detected.
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if(motorFd || (tv.tv_sec < motorLastMovedTime + 3)) result->tm_wday = 8;
  return result;
}
