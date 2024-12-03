#include <chrono>
#include <thread>

#include "hal/time/time.h"

namespace tvsc::hal::time {

void delay_ms(TimeType milliseconds) {
  std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::milli>(milliseconds));
}

void delay_us(TimeType microseconds) {
  std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::micro>(microseconds));
}

TimeType time_millis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

TimeType time_micros() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

}  // namespace tvsc::hal::time
