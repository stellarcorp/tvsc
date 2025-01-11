#pragma once

#include "hal/boards/nucleo_h743zi/rcc.h"
#include "hal/time/clock.h"
#include "hal/time/time.h"

namespace tvsc::hal::boards::nucleo_h743zi {

/**
 * Class to manage the compute the current time and handle sleep requests for the STM32H7xx.
 */
class ClockStm32H7xx final : public time::Clock {
 private:
  const Rcc* const rcc_;

 public:
  ClockStm32H7xx(const Rcc& rcc) : rcc_(&rcc) {}

  time::TimeType current_time_millis() override {
    return static_cast<time::TimeType>(rcc_->tick()) * 1000 / rcc_->tick_frequency();
  }
  time::TimeType current_time_micros() override {
    return static_cast<time::TimeType>(rcc_->tick()) * 1e6 / rcc_->tick_frequency();
  }

  void sleep_ms(time::TimeType milliseconds) override {
    time::TimeType sleep_until(current_time_millis() + milliseconds);

    while (current_time_millis() < sleep_until) {
      // Do nothing
    }
  }

  void sleep_us(time::TimeType microseconds) override {
    time::TimeType sleep_until(current_time_micros() + microseconds);

    while (current_time_micros() < sleep_until) {
      // Do nothing
    }
  }
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
