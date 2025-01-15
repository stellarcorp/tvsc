#include "hal/boards/nucleo_h743zi/ticker.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

uint32_t tick_frequency{0};

volatile CTimeType current_time_us{0};

void SysTick_Handler() { current_time_us += 1e2; }

#ifdef __cplusplus
}
#endif
