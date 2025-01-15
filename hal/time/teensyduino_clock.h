#pragma once

#include "hal/time/clock.h"

namespace tvsc::hal::time {

class TeensyduinoClock final : public Clock {
 public:
  TimeType current_time_millis() override;
  TimeType current_time_micros() override;

  void sleep_ms(TimeType milliseconds) override;
  void sleep_us(TimeType microseconds) override;
};

}  // namespace tvsc::hal::time
