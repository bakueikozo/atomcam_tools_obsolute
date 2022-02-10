#include <stdio.h>
#include <dlfcn.h>
#include <string.h>

static int (*original_local_sdk_motor_move)(unsigned int p1,unsigned int p2,int p3,void *p4, void *p5,unsigned int p6);
static int (*original_local_sdk_motor_move_abs_step)(unsigned int p1,unsigned int p2,int p3,void *p4, void *p5,unsigned int p6);
//static int (*original_local_sdk_motor_move_abs_angle)(float p1,float p2,void *p3 ,void *p4 ,int p5, void *p6,void *p7, void *p8);

static void __attribute ((constructor)) motor_init(void) {

  original_local_sdk_motor_move = dlsym(dlopen ("/system/lib/liblocalsdk_motor.so", RTLD_LAZY), "local_sdk_motor_move");
  original_local_sdk_motor_move_abs_step = dlsym(dlopen ("/system/lib/liblocalsdk_motor.so", RTLD_LAZY), "local_sdk_motor_move_abs_step");
 // original_local_sdk_motor_move_abs_angle = dlsym(dlopen ("/system/lib/liblocalsdk_motor.so", RTLD_LAZY), "local_sdk_motor_move_abs_angle");
}

int local_sdk_motor_move(unsigned int p1,unsigned int p2,int p3,void *p4, void *p5,unsigned int p6) {

  fprintf(stderr, "local_sdk_motor_move(%d, %d, %d, %08x, %08x, %d\n", p1, p2, p3, p4, p5, p6);
  int stat = original_local_sdk_motor_move(p1, p2, p3, p4, p5, p6);
  fprintf(stderr, "local_sdk_motor_move ret = %d\n", stat);
  return stat;
}

int local_sdk_motor_move_abs_step(unsigned int p1,unsigned int p2,int p3,void *p4, void *p5,unsigned int p6) {

  fprintf(stderr, "local_sdk_motor_move_abs_step(%d, %d, %d, %08x, %08x, %d\n", p1, p2, p3, p4, p5, p6);
  int stat = original_local_sdk_motor_move_abs_step(p1, p2, p3, p4, p5, p6);
  fprintf(stderr, "local_sdk_motor_move_abs_step ret = %d\n", stat);
  return stat;
}

#if 0
int local_sdk_motor_move_abs_angle(float p1,float p2,void *p3 ,void *p4 ,int p5, void *p6,void *p7, void *p8) {

  fprintf(stderr, "local_sdk_motor_move_abs_angle(%f, %f, %08x, %08x, %08x, %08x, %08x, %08x)\n", p1, p2, p3, p4, p5, p6, p7, p8);
  int stat = original_local_sdk_motor_move_abs_angle(p1, p2, p3, p4, p5, p6, p7, p8);
  fprintf(stderr, "local_sdk_motor_move_abs_angle ret = %d\n", stat);
  return stat;
}
#endif
