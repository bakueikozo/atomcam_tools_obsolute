#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <pthread.h>
#include <time.h>

static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static uint32_t (*real_local_sdk_video_set_encode_frame_callback)(uint32_t param1,uint32_t param2)=NULL;
static uint32_t (*real_local_sdk_video_set_yuv_frame_callback)(uint32_t param1,uint32_t param2)=NULL;
typedef uint32_t (* framecb)(uint32_t);

typedef struct tm * (*gmtime_r_t)(const time_t *, struct tm *);

void *pfunccb=NULL;
void *pfuncycb=NULL;
int cnt=0;
struct v4l2_capability vid_caps;
struct v4l2_format vid_format;
int v4l2_fd;
char v4l2_device_path[255];

struct v4l2_capability vid_caps2;
struct v4l2_format vid_format2;
int v4l2_fd2;
char v4l2_device_path2[255];

int cnt2=0;
static uint32_t test_capture(void *param){
    int ret=0;
    strcpy(v4l2_device_path,"/dev/video1");
    // param is malloc'd pointer
    if( cnt==0 ){
        fprintf(stderr,"Opening V4L2 device: %s \n", v4l2_device_path);
        v4l2_fd=open(v4l2_device_path, O_WRONLY, 0777);

        if (v4l2_fd < 0) {
            fprintf(stderr,"Failed to open V4L2 device: %s\n", v4l2_device_path);
            //return -1;
        }
        memset(&vid_format, 0, sizeof(vid_format));
        vid_format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        vid_format.fmt.pix.width = 1920;
        vid_format.fmt.pix.height = 1080;

        vid_format.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
        vid_format.fmt.pix.sizeimage = 0;
        vid_format.fmt.pix.field = V4L2_FIELD_NONE;
        vid_format.fmt.pix.bytesperline = 0;
        vid_format.fmt.pix.colorspace = V4L2_PIX_FMT_YUV420;

        ret = ioctl(v4l2_fd, VIDIOC_S_FMT, &vid_format);
        if (ret < 0) {
            fprintf(stderr,"Unable to set V4L2 device video format: %d\n", ret);
            //return -1;
        }

        ret = ioctl(v4l2_fd, VIDIOC_STREAMON, &vid_format);
        if (ret < 0) {
            fprintf(stderr,"Unable to perform VIDIOC_STREAMON: %d\n", ret);
            //return -1;
        }
    }


    uint32_t *ptr=(uint32_t *)param;
    uint32_t length=ptr[1];
    ret = write(v4l2_fd, (void *)(*(uint32_t*)param), length);
    if (ret != length) {
        fprintf(stderr,"Stream write error: %s\n", ret);
    }
    cnt++;
    ret=((framecb)pfunccb)((uint32_t)param);
    //fprintf(stderr,"callback enc frame ret=%d\n",ret);
    return ret;
}
static uint32_t test_captureyuv(void *param){
    int ret=0;
    strcpy(v4l2_device_path2,"/dev/video2");
    // param is malloc'd pointer
    if( cnt2==0 ){
        fprintf(stderr,"Opening V4L2 device: %s \n", v4l2_device_path2);
        v4l2_fd2=open(v4l2_device_path2, O_WRONLY, 0777);

        if (v4l2_fd2 < 0) {
            fprintf(stderr,"Failed to open V4L2 device: %s\n", v4l2_device_path2);
            //return -1;
        }
        memset(&vid_format2, 0, sizeof(vid_format2));
        vid_format2.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        vid_format2.fmt.pix.width = 1920;
        vid_format2.fmt.pix.height = 1080;

        vid_format2.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
        vid_format2.fmt.pix.sizeimage = 1920*1080*2;
        vid_format2.fmt.pix.field = V4L2_FIELD_NONE;
        vid_format2.fmt.pix.bytesperline = 0;
        vid_format2.fmt.pix.colorspace = V4L2_PIX_FMT_YUV420;

        ret = ioctl(v4l2_fd2, VIDIOC_S_FMT, &vid_format2);
        if (ret < 0) {
            fprintf(stderr,"Unable to set V4L2 device video format: %d\n", ret);
            //return -1;
        }

        ret = ioctl(v4l2_fd2, VIDIOC_STREAMON, &vid_format2);
        if (ret < 0) {
            fprintf(stderr,"Unable to perform VIDIOC_STREAMON: %d\n", ret);
            //return -1;
        }
    }


    uint32_t *ptr=(uint32_t *)param;
    uint32_t length=ptr[1];
    ret = write(v4l2_fd2, (void *)(*(uint32_t*)param), length);
    if(cnt2==0){
        fprintf(stderr,"yuv frame length=%d\n", length);

    }
    if (ret != length) {
        fprintf(stderr,"Stream write error: %s\n", ret);
    }
    cnt2++;
    
    return 0;
}
static void *jpg_stream_thread(void *m)
{
    int n=0;
    char filename[255];
    fprintf(stderr,"jpeg_stream_thread_start");
    while(1){
        FILE *fp;
        fp=fopen("/tmp/get_jpeg", "r");
        if( fp ){
            sprintf(filename,"/tmp/snapshot.jpg");
            local_sdk_video_get_jpeg(0,filename);
            remove("/tmp/get_jpeg");
            fclose(fp);
            sleep(0);
        }else{
            
        }
    }

}
uint32_t local_sdk_video_set_encode_frame_callback(uint32_t param1,uint32_t param2){
    void *handle;
	fprintf(stderr,"!!! called local_sdk_video_set_encode_frame_callback !!!\n");
	fflush(stderr);

    fprintf(stderr,"param1 : stream ch id=%d\n",param1);
    if (real_local_sdk_video_set_encode_frame_callback == NULL){
        handle = dlopen ("/system/lib/liblocalsdk.so", RTLD_LAZY);
        if (!handle) {
            fputs (dlerror(), stderr);
        }
        real_local_sdk_video_set_encode_frame_callback = dlsym(handle, "local_sdk_video_set_encode_frame_callback");
        fprintf(stderr,"ptr=%d injected!!! err=%s\n",real_local_sdk_video_set_encode_frame_callback,dlerror());
	}
    fprintf(stderr,"param1=0x%x,param2=0x%x,*param2=0x%x\n",param1,param2,*(int32_t*)param2);
    if(param1==0){
        pfunccb=param2;
        fprintf(stderr,"enc func injection save pcb=0x%x\n",pfunccb);
        param2=(uint32_t)test_capture;
        fprintf(stderr,"override to 0x%x\n",param2);
        pthread_t tid; /* Stream capture in another thread */
    	pthread_create(&tid, NULL, jpg_stream_thread, NULL);
    }
    int ret=real_local_sdk_video_set_encode_frame_callback(param1,param2);
/*
    if( param1==1){
        fprintf(stderr,"try to set yuv frame callback\n");
        local_sdk_video_set_yuv_frame_callback(1,(uint32_t)test_captureyuv);
    }*/
	return ret;
}
