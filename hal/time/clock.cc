#include "hal/time/clock.h"

namespace tvsc::hal::time {

Clock::time_point Clock::now() noexcept {
  return time_point{duration{system_clock().current_time_micros()}};
}

}  // namespace tvsc::hal::time
