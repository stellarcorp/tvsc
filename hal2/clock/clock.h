#pragma once

#include <chrono>

#include "hal2/rcc/rcc.h"
#include "hal2/systick/systick.h"
#include "hal2/time_type.h"
#include "hal2/timer/timer.h"

namespace tvsc::hal2::clock {

/**
 * Class to manage the compute the current time and handle sleep requests. This class is modelled
 * after the C++ Clock named requirement, but it does not include static accessors like the now()
 * method.
 */
class Clock final {
 private:
  systick::SysTickType* systick_;
  rcc::Rcc* rcc_;
  // Low-power timer. Note that we are keeping this timer on the whole time, since we expect to use
  // it often.
  timer::Timer timer_;

 public:
  // C++ Clock types.
  using rep = TimeType;
  using period = std::micro;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<Clock, duration>;

  // This clock always moves forward.
  static constexpr bool is_steady{true};

  Clock(systick::SysTickType& systick,  //
        rcc::Rcc& rcc,                  //
        timer::TimerPeripheral& timer_peripheral) noexcept
      : systick_(&systick), rcc_(&rcc), timer_(timer_peripheral.access()) {}

  [[nodiscard]] TimeType current_time_micros() const noexcept {
    return systick_->current_time_micros();
  }

  [[nodiscard]] TimeType current_time_millis() const noexcept {
    return systick_->current_time_micros() / 1000;
  }

  [[nodiscard]] time_point current_time() const noexcept {
    return time_point{std::chrono::microseconds{current_time_micros()}};
  }

  void sleep_us(TimeType microseconds) noexcept;
  void sleep_ms(TimeType milliseconds) noexcept { sleep_us(milliseconds * 1000); }

  template <typename Rep, typename Period>
  void sleep(std::chrono::duration<Rep, Period> d) noexcept {
    sleep_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  }

  void sleep(time_point t) noexcept { sleep(t - current_time()); }

  void wait_us(TimeType microseconds) noexcept;
  void wait_ms(TimeType milliseconds) noexcept { wait_us(milliseconds * 1000); }

  template <typename Rep, typename Period>
  void wait(std::chrono::duration<Rep, Period> d) noexcept {
    wait_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  }

  void wait(time_point t) noexcept { wait(t - current_time()); }
};

}  // namespace tvsc::hal2::clock
