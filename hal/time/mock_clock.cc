#include "hal/time/mock_clock.h"

#include <cstdint>

namespace tvsc::hal::time {

uint64_t MockClock::current_time_millis() { return current_time_ms_; }
void MockClock::set_current_time_millis(uint64_t current_time_ms) {
  current_time_ms_ = current_time_ms;
}

void MockClock::sleep_ms(uint32_t t) { current_time_ms_ += t; }
void MockClock::sleep_us(uint32_t t) {
  if (t < 1000) {
    // Force a minimum sleep time of 1ms, otherwise, this call will often behave like a no-op and
    // cause confusion.
    current_time_ms_ += 1;
  } else {
    current_time_ms_ += t / 1000;
  }
}

}  // namespace tvsc::hal::time
