#include "hal/time/std_cpp_clock.h"

#include <chrono>
#include <thread>

namespace tvsc::hal::time {

StdCppClock& StdCppClock::clock() noexcept {
  static StdCppClock clock{};
  return clock;
}

StdCppClock::time_point StdCppClock::now() noexcept { return clock().current_time(); }

TimeType StdCppClock::current_time_millis() noexcept {
  return std::chrono::duration_cast<std::chrono::milliseconds>(current_time().time_since_epoch())
      .count();
}

TimeType StdCppClock::current_time_micros() noexcept {
  return std::chrono::duration_cast<std::chrono::microseconds>(current_time().time_since_epoch())
      .count();
}

void StdCppClock::sleep_ms(TimeType milliseconds) noexcept {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void StdCppClock::sleep_us(TimeType microseconds) noexcept {
  std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
}

}  // namespace tvsc::hal::time
