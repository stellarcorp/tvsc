#pragma once

#include <cstdint>

#include "hal/time/time.h"

namespace tvsc::hal::time {

class Clock {
 public:
  virtual ~Clock() = default;

  virtual TimeType current_time_millis();
  virtual TimeType current_time_micros();

  virtual void sleep_ms(TimeType milliseconds);
  virtual void sleep_us(TimeType microseconds);
};

Clock& default_clock();

}  // namespace tvsc::hal::time
