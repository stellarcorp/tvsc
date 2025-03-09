#pragma once

#include <chrono>

#include "hal/power/power.h"
#include "hal/rcc/rcc.h"
#include "hal/systick/systick.h"
#include "hal/time_type.h"
#include "hal/timer/timer.h"

namespace tvsc::time {

/**
 * Class to manage the compute the current time and handle sleep requests for the STM32 MCUs.
 */
class EmbeddedClock final {
 private:
  tvsc::hal::systick::SysTickType* sys_tick_{};
  // Note that we are keeping this timer on the whole time, since we expect to use it often.
  tvsc::hal::timer::Timer timer_;
  tvsc::hal::power::Power* power_peripheral_;
  tvsc::hal::rcc::Rcc* rcc_;

  EmbeddedClock(tvsc::hal::systick::SysTickType& sys_tick,            //
                tvsc::hal::timer::TimerPeripheral& timer_peripheral,  //
                tvsc::hal::power::Power& power_peripheral,            //
                tvsc::hal::rcc::Rcc& rcc) noexcept
      : sys_tick_(&sys_tick),
        timer_(timer_peripheral.access()),
        power_peripheral_(&power_peripheral),
        rcc_(&rcc) {}

 public:
  // C++ Clock types.
  using rep = tvsc::hal::TimeType;
  using period = std::micro;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<EmbeddedClock, duration>;

  // This clock always moves forward and is never adjusted.
  static constexpr bool is_steady{true};

  [[nodiscard]] static time_point now() noexcept;
  [[nodiscard]] static EmbeddedClock& clock() noexcept;

  [[nodiscard]] tvsc::hal::TimeType current_time_micros() noexcept {
    return sys_tick_->current_time_micros();
  }

  [[nodiscard]] tvsc::hal::TimeType current_time_millis() noexcept {
    return sys_tick_->current_time_micros() / 1000;
  }

  [[nodiscard]] time_point current_time() noexcept {
    return time_point{std::chrono::microseconds{current_time_micros()}};
  }

  void sleep_us(tvsc::hal::TimeType microseconds) noexcept;
  void sleep_ms(tvsc::hal::TimeType milliseconds) noexcept { sleep_us(milliseconds * 1000); }

  template <typename Rep, typename Period>
  void sleep(std::chrono::duration<Rep, Period> d) noexcept {
    sleep_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  }

  void sleep(time_point t) noexcept { sleep(t - current_time()); }
};

}  // namespace tvsc::time
