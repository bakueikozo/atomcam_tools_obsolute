#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/time.h>

typedef void CURL;
typedef int CURLcode;

struct DynamicStatic {
  char *url;
};
struct SessionHandle {
  unsigned char padding[2720];
  struct DynamicStatic change;
};

static const int CURLE_OPERATION_TIMEDOUT = 28;
static const char *AlarmPath = "/device/v1/alarm/add";
static CURLcode (*original_curl_easy_perform)(CURL *curl);

static void __attribute ((constructor)) curl_hook_init(void) {

  original_curl_easy_perform = dlsym(dlopen ("/thirdlib/libcurl.so", RTLD_LAZY), "curl_easy_perform");
}

CURLcode curl_easy_perform(struct SessionHandle *data) {

  if(data->change.url && !strcmp(data->change.url + strlen(data->change.url) - strlen(AlarmPath), AlarmPath)) {
    static time_t lastAccess = 0;
    struct timeval now;
    gettimeofday(&now, NULL);
    printf("[curl-debug] easy_perform %s\n", data->change.url);
    if(now.tv_sec - lastAccess < 300) {
      printf("[curl-debug] Dismiss short cycle alarms.\n");
      return CURLE_OPERATION_TIMEDOUT;
    }
    CURLcode res = original_curl_easy_perform(data);
    printf("[curl-debug] easy_perform res=%d\n", res);
    if(!res) lastAccess = now.tv_sec;
    return res;
  }
  return original_curl_easy_perform(data);
}

