#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

extern int local_sdk_motor_get_position(float *step,float *angle);
extern int local_sdk_motor_move_abs_angle(float pan, float tilt, int speed, void (*done)(float a, float b), void (*canceled)(void), int mode);
extern void CommandResponse(int fd, const char *res);

int motorFd = 0;
int motorLastMovedTime = 0;

static void motor_move_done(float pan, float tilt) {

  if(motorFd) {
    static char motorResBuf[256];
    sprintf(motorResBuf, "%f %f\n", pan, tilt);
    CommandResponse(motorFd, motorResBuf);
  }
  motorFd = 0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  motorLastMovedTime = tv.tv_sec;
}

static void motor_move_canceled() {

  if(motorFd) CommandResponse(motorFd, "error");
  motorFd = 0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  motorLastMovedTime = tv.tv_sec;
}

char *MotorMove(int fd, char *tokenPtr) {

  char *p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) {
    float pan; // 0-355
    float tilt; // 0-180
    int ret = local_sdk_motor_get_position(&pan, &tilt);
    static char motorResBuf[256];
    sprintf(motorResBuf, "%f %f\n", pan, tilt);
    return motorResBuf;
  }
  float pan = atof(p); // 0-355
  if((pan < 0.0) || (pan > 355.0)) return "error";

  p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  if(!p) return "error";
  float tilt = atof(p); // 0-180
  if((tilt < 0.0) || (tilt > 180.0)) return "error";

  p = strtok_r(NULL, " \t\r\n", &tokenPtr);
  int pri = 2; // 0: high - 3: low
  if(p) pri = atoi(p);
  if(pri < 0) pri = 0;
  if(pri > 3) pri = 3;

  if(motorFd) return "error";
  motorFd = fd;

  int speed = 9;
  int res = local_sdk_motor_move_abs_angle(pan, tilt, speed, &motor_move_done, &motor_move_canceled, pri);
  return NULL;
}
