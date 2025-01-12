#include "hal/boards/nucleo_h743zi/ticker.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

uint32_t tick_frequency{};

CTimeType current_time_us{};

void SysTick_Handler() { current_time_us += tick_frequency * 1e6 / SystemCoreClock; }

#ifdef __cplusplus
}
#endif
