#pragma once

#include "hal/time/clock.h"

namespace tvsc::hal::time {

/**
 * Class to manage the compute the current time and handle sleep requests for the STM32H7xx.
 */
class ClockStm32xxxx final : public time::Clock {
 public:
  TimeType current_time_micros() override;
  TimeType current_time_millis() override;

  void sleep_us(TimeType microseconds) override;
  void sleep_ms(TimeType milliseconds) override;
};

}  // namespace tvsc::hal::time
