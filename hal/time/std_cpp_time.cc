#include <chrono>
#include <thread>

#include "hal/time/time.h"

namespace tvsc::hal::time {

void delay_ms(uint32_t milliseconds) {
  std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::milli>(milliseconds));
}

void delay_us(uint32_t microseconds) {
  std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::micro>(microseconds));
}

uint64_t time_millis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

}  // namespace tvsc::hal::time
