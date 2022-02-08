#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <pthread.h>

extern int video_enable;
extern int jpeg_capture;
extern void local_sdk_video_get_jpeg(int, char *);
extern void commandResponse(int fd, const char *res);

struct frames_st {
  void *buf;
  size_t length;
};
typedef int (* framecb)(struct frames_st *);

static uint32_t (*real_local_sdk_video_set_encode_frame_callback)(int ch, void *callback);
static void *video_encode_cb = NULL;

static void __attribute ((constructor)) video_callback_init(void) {
  real_local_sdk_video_set_encode_frame_callback = dlsym(dlopen("/system/lib/liblocalsdk.so", RTLD_LAZY), "local_sdk_video_set_encode_frame_callback");
}

static uint32_t video_encode_capture(struct frames_st *frames) {
  uint32_t ret;
  static int firstEntry = 0;
  static int v4l2Fd = -1;

  if(!firstEntry) {
    firstEntry++;
    int err;
    const char *v4l2_device_path = "/dev/video1";
    fprintf(stderr,"Opening V4L2 device: %s \n", v4l2_device_path);
    v4l2Fd = open(v4l2_device_path, O_WRONLY, 0777);
    if(v4l2Fd < 0) fprintf(stderr,"Failed to open V4L2 device: %s\n", v4l2_device_path);
    struct v4l2_format vid_format;
    memset(&vid_format, 0, sizeof(vid_format));
    vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    vid_format.fmt.pix.width = 1920;
    vid_format.fmt.pix.height = 1080;
    vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
    vid_format.fmt.pix.sizeimage = 0;
    vid_format.fmt.pix.field = V4L2_FIELD_NONE;
    vid_format.fmt.pix.bytesperline = 0;
    vid_format.fmt.pix.colorspace = V4L2_PIX_FMT_YUV420;
    err = ioctl(v4l2Fd, VIDIOC_S_FMT, &vid_format);
    if(err < 0) fprintf(stderr,"Unable to set V4L2 device video format: %d\n", err);
    err = ioctl(v4l2Fd, VIDIOC_STREAMON, &vid_format);
    if(err < 0) fprintf(stderr,"Unable to perform VIDIOC_STREAMON: %d\n", err);
  }

  if(jpeg_capture) {
    local_sdk_video_get_jpeg(0, "/tmp/snapshot.jpg");
    commandResponse(jpeg_capture, "ok");
    jpeg_capture = 0;
  }

  if( (v4l2Fd >= 0) && video_enable) {
    uint32_t *buf = frames->buf;
    int size = write(v4l2Fd, frames->buf, frames->length);
    if(size != frames->length) fprintf(stderr,"Stream write error: %s\n", ret);
  }
  return ((framecb)video_encode_cb)(frames);
}

uint32_t local_sdk_video_set_encode_frame_callback(int ch, void *callback) {
  fprintf(stderr, "local_sdk_video_set_encode_frame_callback streamChId=%d, callback=0x%x\n", ch, callback);
  if(ch == 0) {
    video_encode_cb = callback;
    fprintf(stderr,"enc func injection save video_encode_cb=0x%x\n", video_encode_cb);
    callback = video_encode_capture;
  }
  return real_local_sdk_video_set_encode_frame_callback(ch, callback);
}
