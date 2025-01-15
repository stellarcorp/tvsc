#include "hal/time/std_cpp_clock.h"

#include <chrono>
#include <thread>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

TimeType StdCppClock::current_time_millis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

TimeType StdCppClock::current_time_micros() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

void StdCppClock::sleep_ms(TimeType milliseconds) {
  std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::milli>(milliseconds));
}

void StdCppClock::sleep_us(TimeType microseconds) {
  std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::micro>(microseconds));
}

}  // namespace tvsc::hal::time
