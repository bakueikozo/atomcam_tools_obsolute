#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>

static uint32_t (*real_local_sdk_video_set_encode_frame_callback)(uint32_t param1, uint32_t param2);
extern void local_sdk_video_get_jpeg(int, char *);
typedef uint32_t (* framecb)(uint32_t);
void *pfunccb = NULL;
struct v4l2_format vid_format;
const char *v4l2_device_path = "/dev/video1";

static void __attribute ((constructor)) filesnooper_init(void) {
  real_local_sdk_video_set_encode_frame_callback = dlsym(dlopen("/system/lib/liblocalsdk.so", RTLD_LAZY), "local_sdk_video_set_encode_frame_callback");
}

static uint32_t test_capture(void *param) {
  uint32_t ret;
  static int firstEntry = 0;
  static int v4l2Fd = -1;

  if(!firstEntry) {
    int err;
    firstEntry++;
    fprintf(stderr,"Opening V4L2 device: %s \n", v4l2_device_path);
    v4l2Fd = open(v4l2_device_path, O_WRONLY, 0777);
    if(v4l2Fd < 0) fprintf(stderr,"Failed to open V4L2 device: %s\n", v4l2_device_path);
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

  FILE *fp = fopen("/tmp/get_jpeg", "r");
  if(fp) {
    local_sdk_video_get_jpeg(0, "/tmp/snapshot.jpg");
    remove("/tmp/get_jpeg");
    fclose(fp);
  }

  if(v4l2Fd >= 0) {
    uint32_t *ptr = (uint32_t *)param;
    uint32_t length = ptr[1];
    int size = write(v4l2Fd, (void *)(*(uint32_t*)param), length);
    if(size != length) fprintf(stderr,"Stream write error: %s\n", ret);
  }
  return ((framecb)pfunccb)((uint32_t)param);
}

uint32_t local_sdk_video_set_encode_frame_callback(uint32_t param1, uint32_t param2) {
  fprintf(stderr, "local_sdk_video_set_encode_frame_callback streamChId=%d, param2=0x%x, *param2=0x%x\n", param1, param2, *(int32_t*)param2);
  if(param1 == 0) {
    pfunccb = (void *)param2;
    fprintf(stderr,"enc func injection save pcb=0x%x\n", pfunccb);
    param2 = (uint32_t)test_capture;
    fprintf(stderr,"override to 0x%x\n", param2);
  }
  return real_local_sdk_video_set_encode_frame_callback(param1, param2);
}
