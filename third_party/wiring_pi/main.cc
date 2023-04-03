#include "wiringPi.h"

#ifdef __cpluplus
extern "C" {
#endif

extern void setup();
extern void loop();

#ifdef __cpluplus
}
#endif

int main(int argc, char* argv[]) {
  wiringPiSetupGpio();

  setup();

  while (true) {
    loop();
  }

  return 0;
}
