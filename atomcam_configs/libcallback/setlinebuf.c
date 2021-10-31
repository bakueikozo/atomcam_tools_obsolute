#include <stdio.h>
static void __attribute ((constructor)) setStdoutLineBuffer(void) {
  setlinebuf(stdout);
}
