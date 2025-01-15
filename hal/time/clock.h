#pragma once

#include <cstdint>

namespace tvsc::hal::time {

// Type for representing times (usually a count of seconds, milliseconds, or microseconds) as
// scalars.
using TimeType = uint64_t;

class Clock {
 public:
  virtual ~Clock() = default;

  virtual TimeType current_time_millis() = 0;
  virtual TimeType current_time_micros() = 0;

  virtual void sleep_ms(TimeType milliseconds) = 0;
  virtual void sleep_us(TimeType microseconds) = 0;
};

}  // namespace tvsc::hal::time
