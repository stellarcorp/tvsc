#pragma once

#include <cstdint>

namespace tvsc::hal::time {

class Clock {
 public:
  virtual ~Clock() = default;

  virtual uint64_t current_time_millis();

  virtual void sleep_ms(uint32_t milliseconds);
  virtual void sleep_us(uint32_t microseconds);
};

Clock& default_clock();

}  // namespace tvsc::hal::time
