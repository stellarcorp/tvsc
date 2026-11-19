#pragma once

#include <chrono>

#include "hal2/time_type.h"

namespace tvsc::hal2::clock {

/**
 * Class to manage the compute the current time and handle sleep requests. This class is modelled
 * after the C++ Clock named requirement, but it does not include static accessors like the now()
 * method.
 */
class Clock {
 public:
  // C++ Clock types.
  using rep = TimeType;
  using period = std::micro;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<Clock, duration>;

  // This clock always moves forward.
  static constexpr bool is_steady{true};

  virtual ~Clock() = default;

  [[nodiscard]] virtual TimeType current_time_micros() const noexcept = 0;
  virtual void sleep_us(TimeType microseconds) noexcept = 0;
  virtual void wait_us(TimeType microseconds) noexcept = 0;

  [[nodiscard]] TimeType current_time_millis() const noexcept {
    return current_time_micros() / 1000;
  }

  [[nodiscard]] time_point current_time() const noexcept {
    return time_point{std::chrono::microseconds{current_time_micros()}};
  }

  void sleep_ms(TimeType milliseconds) noexcept { sleep_us(milliseconds * 1000); }

  template <typename Rep, typename Period>
  void sleep(std::chrono::duration<Rep, Period> d) noexcept {
    sleep_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  }

  void sleep(time_point t) noexcept { sleep(t - current_time()); }

  void wait_ms(TimeType milliseconds) noexcept { wait_us(milliseconds * 1000); }

  template <typename Rep, typename Period>
  void wait(std::chrono::duration<Rep, Period> d) noexcept {
    wait_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  }

  void wait(time_point t) noexcept { wait(t - current_time()); }
};

}  // namespace tvsc::hal2::clock
