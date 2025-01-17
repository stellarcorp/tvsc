#include "hal/time/stm_clock.h"

#include "hal/time/clock.h"

namespace tvsc::hal::time {

time::TimeType ClockStm32xxxx::current_time_micros() { return *current_time_us_; }
time::TimeType ClockStm32xxxx::current_time_millis() { return *current_time_us_ / 1000; }

void ClockStm32xxxx::sleep_us(time::TimeType microseconds) {
  const time::TimeType sleep_until_us{current_time_micros() + microseconds};
  while (current_time_micros() < sleep_until_us) {
    // Do nothing
  }
}

void ClockStm32xxxx::sleep_ms(time::TimeType milliseconds) { sleep_us(milliseconds * 1000); }

}  // namespace tvsc::hal::time
