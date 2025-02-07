#include "hal/time/stm_clock.h"

#include <cstdint>

#include "hal/time/clock.h"

#ifdef __cplusplus
extern "C" {
#endif

extern volatile uint64_t uwTick;

void SysTick_Handler() { ++uwTick; }

#ifdef __cplusplus
}
#endif

namespace tvsc::hal::time {

Clock& system_clock() {
  static ClockStm32xxxx system_clock_{};
  return system_clock_;
}

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
