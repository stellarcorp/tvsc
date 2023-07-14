#include "hal/time/clock.h"

#include <cstdint>

#include "hal/time/time.h"

namespace tvsc::hal::time {

uint64_t Clock::current_time_millis() { return time_millis(); }

Clock& default_clock() {
  static Clock clock{};
  return clock;
}

}  // namespace tvsc::hal::time
