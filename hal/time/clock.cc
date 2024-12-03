#include "hal/time/clock.h"

#include <cstdint>

#include "hal/time/time.h"

namespace tvsc::hal::time {

TimeType Clock::current_time_millis() { return time_millis(); }
TimeType Clock::current_time_micros() { return time_micros(); }

void Clock::sleep_ms(TimeType t) { delay_ms(t); }
void Clock::sleep_us(TimeType t) { delay_us(t); }

Clock& default_clock() {
  static Clock clock{};
  return clock;
}

}  // namespace tvsc::hal::time
