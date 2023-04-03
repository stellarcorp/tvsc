#include <chrono>
#include <thread>

#include "hal/gpio/time.h"

namespace tvsc::hal::gpio {

void delay_ms(uint32_t milliseconds) {
  std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::milli>(milliseconds));
}

void delay_us(uint32_t microseconds) {
  std::this_thread::sleep_for(std::chrono::duration<uint32_t, std::micro>(microseconds));
}

}  // namespace tvsc::hal::gpio
