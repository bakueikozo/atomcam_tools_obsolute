#include <stdio.h>

// disable usb power control
void local_sdk_usb_power_on(void) {
  printf("skip local_sdk_usb_power_on\n");
}

void local_sdk_usb_power_off(void) {
  printf("skip local_sdk_usb_power_off\n");
}
