#pragma once

#include "hal/power/power.h"
#include "hal/rcc/rcc.h"
#include "hal/time/clock.h"
#include "hal/timer/timer.h"

namespace tvsc::hal::time {

/**
 * Class to manage the compute the current time and handle sleep requests for the STM32H7xx.
 */
class ClockStm32xxxx final : public Clock {
 private:
  timer::TimerPeripheral* timer_peripheral_;

  // Note that we are keeping this timer on the whole time. We might be better served by turning it
  // on and off inside of sleep_us().
  timer::Timer timer_{timer_peripheral_->access()};
  power::Power* power_peripheral_;
  rcc::Rcc* rcc_;

 public:
  ClockStm32xxxx(timer::TimerPeripheral& timer_peripheral, power::Power& power_peripheral,
                 rcc::Rcc& rcc)
      : timer_peripheral_(&timer_peripheral), power_peripheral_(&power_peripheral), rcc_(&rcc) {}

  TimeType current_time_micros() override;
  TimeType current_time_millis() override;

  void sleep_us(TimeType microseconds) override;
  void sleep_ms(TimeType milliseconds) override;
};

}  // namespace tvsc::hal::time
