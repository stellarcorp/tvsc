#pragma once

#include <chrono>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

/**
 * std::chrono::steady_clock with extra methods for sleeping.
 */
class StdCppClock final {
 public:
  // C++ Clock types.
  using rep = std::chrono::steady_clock::rep;
  using period = std::chrono::steady_clock::period;
  using duration = std::chrono::steady_clock::duration;
  using time_point = std::chrono::time_point<std::chrono::steady_clock, duration>;

  // This clock always moves forward and is never adjusted.
  static constexpr bool is_steady{true};

  [[nodiscard]] TimeType current_time_millis() noexcept;
  [[nodiscard]] TimeType current_time_micros() noexcept;
  [[nodiscard]] time_point current_time() noexcept { return std::chrono::steady_clock::now(); }

  void sleep_ms(TimeType milliseconds) noexcept;
  void sleep_us(TimeType microseconds) noexcept;

  [[nodiscard]] static time_point now() noexcept;
  [[nodiscard]] static StdCppClock& clock() noexcept;
};

}  // namespace tvsc::hal::time
