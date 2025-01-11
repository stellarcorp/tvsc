#pragma once

#include "hal/time/clock.h"
#include "hal/time/time.h"

namespace tvsc::hal::boards::nucleo_h743zi {

/**
 * Class to manage the compute the current time and handle sleep requests for the STM32H7xx.
 */
class ClockStm32H7xx final : public time::Clock {
 public:
  time::TimeType current_time_millis() override { return 0; }
  time::TimeType current_time_micros() override { return 0; }

  void sleep_ms(time::TimeType milliseconds) override {}
  void sleep_us(time::TimeType microseconds) override {}
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
