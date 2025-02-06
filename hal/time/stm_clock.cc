#include "hal/time/stm_clock.h"

#include "hal/time/clock.h"
#include "third_party/stm32/stm32_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void SysTick_Handler() { uwTick += static_cast<uint32_t>(uwTickFreq); }

#ifdef __cplusplus
}
#endif

namespace tvsc::hal::time {

TimeType ClockStm32xxxx::current_time_micros() { return uwTick * 1000; }
TimeType ClockStm32xxxx::current_time_millis() { return uwTick; }

void ClockStm32xxxx::sleep_us(TimeType microseconds) {
  const TimeType sleep_until_us{current_time_micros() + microseconds};
  while (current_time_micros() < sleep_until_us) {
    // Do nothing
  }
}

void ClockStm32xxxx::sleep_ms(TimeType milliseconds) { sleep_us(milliseconds * 1000); }

}  // namespace tvsc::hal::time
