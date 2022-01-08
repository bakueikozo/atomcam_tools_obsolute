#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

typedef void CURL;
typedef int CURLcode;
typedef long long curl_off_t;

typedef enum {
  HTTPREQ_NONE,
  HTTPREQ_GET,
  HTTPREQ_POST,
  HTTPREQ_POST_FORM,
  HTTPREQ_POST_MIME,
  HTTPREQ_PUT,
  HTTPREQ_HEAD,
  HTTPREQ_OPTIONS,
  HTTPREQ_LAST
} Curl_HttpReq;

static const int CURLE_OK = 0;

const char * methods[] = {
  "NONE", "GET", "POST", "POST_FORM", "POST_MIME", "PUT", "HEAD", "OPTIONS", "LAST", ""
};

static const char *AlarmPath = "/device/v1/alarm/add";

static const char *DummyRes = "{\"ts\":1641390551000,\"code\":\"1\",\"msg\":\"\",\"data\":{\"alarm_file_list\":[{\"file_type\":1,\"file_suffix\":\"jpg\",\"file_url\":\"https://localhost/hoge.jpg\",\"encryption_algorithm\":0,\"encryption_password\":\"\"},{\"file_type\":2,\"file_suffix\":\"mp4\",\"file_url\":\"https://localhost/fuga.mp4\",\"encryption_algorithm\":0,\"encryption_password\":\"\"}]}}";

static const char *DummyHost = "https://localhost/";

typedef int (*curl_seek_callback)(void *instream,
                                  int offset,
                                  int origin); /* 'whence' */

typedef int (*curl_write_callback)(char *buffer,
                                      int size,
                                      int nitems,
                                      void *outstream);

struct UserDefined {
  FILE *err;
  void *debugdata;
  char *errorbuffer;
  long proxyport;
  void *out;
  void *in_set;
  void *writeheader;
  void *rtp_out;
  long use_port;
  unsigned long httpauth;
  unsigned long proxyauth;
  unsigned long socks5auth;
  long followlocation;
  long maxredirs;
  int keep_post;
  void *postfields;
  void *seek_func;
  curl_off_t postfieldsize;
  unsigned short localport;
  int localportrange;
  curl_write_callback fwrite_func;
  void *fwrite_header;
  void *fwrite_rtp;
  void *fread_func_set;
  void *fprogress;
  void *fxferinfo;
  void *fdebug;
  void *ioctl_func;
  void *fsockopt;
  void *sockopt_client;
  void *fopensocket;
  void *opensocket_client;
  void * fclosesocket;
  void *closesocket_client;
  void *seek_client;
  void * convfromnetwork;
  void * convtonetwork;
  void * convfromutf8;
  void *progress_client;
  void *ioctl_client;
  long timeout;
  long connecttimeout;
  long accepttimeout;
  long happy_eyeballs_timeout;
  long server_response_timeout;
  long maxage_conn;
  long tftp_blksize;
  curl_off_t filesize;
  long low_speed_limit;
  long low_speed_time;
  curl_off_t max_send_speed;
  curl_off_t max_recv_speed;
  curl_off_t set_resume_from;
  void *headers;
  void *proxyheaders;
  void *httppost;
  curl_off_t mimepost_pad;
  unsigned char mimepost[352];
  void *quote;
  void *postquote;
  void *prequote;
  void *source_quote;
  void *source_prequote;
  void *source_postquote;
  void *telnet_options;
  void *resolve;
  void *connect_to;
  int timecondition;
  time_t timevalue;
  Curl_HttpReq httpreq;
  long httpversion;
};

struct DynamicStatic {
  char *url;
  char *referer;
  void *cookielist;
  void *resolve;
  unsigned int bit;
};

struct PureInfo {
  int httpcode;
};

struct SessionHandle {
  unsigned char padding[1392];
  union {
    unsigned char set_bin[1328];
    struct UserDefined set;
  } ud;
  struct DynamicStatic change;
  unsigned char padding2[988];
  struct PureInfo info;
};

static CURLcode (*original_curl_easy_perform)(CURL *curl);
static int curl_hook_enable = 0;
static void __attribute ((constructor)) curl_hook_init(void) {

  original_curl_easy_perform = dlsym(dlopen("/thirdlib/libcurl.so", RTLD_LAZY), "curl_easy_perform");
  char *p = getenv("MINIMIZE_ALARM_CYCLE");
  curl_hook_enable = p && !strcmp(p, "on");
}

static void Dump(const char *str, void *start, int size) {

  printf("[curl-debug] Dump %s\n", str);
  for(int i = 0; i < size; i+= 16) {
    char buf1[256];
    char buf2[256];
    sprintf(buf1, "%08x : ", start + i);
    for(int j = 0; j < 16; j++) {
      if(i + j >= size) break;
      unsigned char d = ((unsigned char *)start)[i + j];
      sprintf(buf1 + strlen(buf1), "%02x ", d);
      if((d < 0x20) || (d > 0x7f)) d = '.';
      sprintf(buf2 + j, "%c", d);
    }
    printf("%s %s\n", buf1, buf2);
  }
}

CURLcode curl_easy_perform(struct SessionHandle *data) {

  if(!curl_hook_enable) return original_curl_easy_perform(data);

  unsigned int ra = 0;
  asm volatile(
    "ori %0, $31, 0\n"
    : "=r"(ra)
  );

  int method = data->ud.set.httpreq;
  if(method > HTTPREQ_LAST) method = HTTPREQ_LAST;
  printf("[curl-debug] %s %s ra=0x%08x\n", methods[method], data->change.url, ra);
  if(data->ud.set.postfields) {
    if(data->ud.set.postfieldsize > 0) {
      Dump("[curl-debug] post", data->ud.set.postfields, data->ud.set.postfieldsize);
    } else {
      printf("[curl-debug] post : %s\n", data->ud.set.postfields);
    }
  }

  if(data->change.url && !strcmp(data->change.url + strlen(data->change.url) - strlen(AlarmPath), AlarmPath)) {
    static time_t lastAccess = 0;
    struct timeval now;
    gettimeofday(&now, NULL);
    if(now.tv_sec - lastAccess < 300) {
      printf("[curl-debug] Dismiss short cycle alarms.\n");
      memcpy(data->ud.set.out, DummyRes, strlen(DummyRes));
      data->info.httpcode = 200;
      return CURLE_OK;
    }
    CURLcode res = original_curl_easy_perform(data);
    printf("[curl-debug] res=%d\n", res);
    if(!res) lastAccess = now.tv_sec;
    return res;
  }

  if(data->change.url && !strncmp(data->change.url, DummyHost, strlen(DummyHost))) {
    printf("[curl-debug] skip http-post.\n");
    data->info.httpcode = 200;
    return CURLE_OK;
  }

  CURLcode res = original_curl_easy_perform(data);
  if(data->ud.set.out) printf("[curl-debug] res : %s\n", data->ud.set.out);
  printf("[curl-debug] ret: %d\n", res);
  return res;
}
