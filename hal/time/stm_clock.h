#pragma once

#include "hal/power/power.h"
#include "hal/time/clock.h"
#include "hal/timer/timer.h"

namespace tvsc::hal::time {

/**
 * Class to manage the compute the current time and handle sleep requests for the STM32H7xx.
 */
class ClockStm32xxxx final : public Clock {
 private:
  timer::TimerPeripheral* timer_peripheral_;
  timer::Timer timer_{};
  power::Power* power_peripheral_;

 public:
  ClockStm32xxxx(timer::TimerPeripheral& timer_peripheral, power::Power& power_peripheral)
      : timer_peripheral_(&timer_peripheral), power_peripheral_(&power_peripheral) {}

  TimeType current_time_micros() override;
  TimeType current_time_millis() override;

  void sleep_us(TimeType microseconds) override;
  void sleep_ms(TimeType milliseconds) override;
};

}  // namespace tvsc::hal::time
