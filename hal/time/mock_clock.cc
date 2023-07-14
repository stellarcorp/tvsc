#include "hal/time/mock_clock.h"

#include <cstdint>

namespace tvsc::hal::time {

uint64_t MockClock::current_time_millis() { return current_time_ms_; }
void MockClock::set_current_time_millis(uint64_t current_time_ms) {
  current_time_ms_ = current_time_ms;
}

}  // namespace tvsc::hal::time
