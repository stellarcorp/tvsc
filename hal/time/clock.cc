#include "hal/time/clock.h"

#include <cstdint>

#include "hal/time/time.h"

namespace tvsc::hal::time {

uint64_t Clock::current_time_millis() { return time_millis(); }
uint64_t Clock::current_time_micros() { return time_micros(); }

void Clock::sleep_ms(uint32_t t) { delay_ms(t); }
void Clock::sleep_us(uint32_t t) { delay_us(t); }

Clock& default_clock() {
  static Clock clock{};
  return clock;
}

}  // namespace tvsc::hal::time
