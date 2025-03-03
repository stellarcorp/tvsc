#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "hal/time/clock.h"
#include "hal/time/clockable.h"

namespace tvsc::hal::time {

template <std::intmax_t SCALE_FACTOR_NUMERATOR, std::intmax_t SCALE_FACTOR_DENOMINATOR = 1,
          typename BaseClockT = std::chrono::steady_clock>
class ScaledClock final {
 public:
  using ClockableType = Clockable<ScaledClock>;
  using BaseClockType = BaseClockT;

  static constexpr double SCALE_FACTOR{static_cast<double>(SCALE_FACTOR_NUMERATOR) /
                                       SCALE_FACTOR_DENOMINATOR};
  // C++ Clock types.
  using rep = double;
  using period = std::micro;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<ScaledClock, duration>;

  // This clock always moves forward and is never adjusted.
  static constexpr bool is_steady{true};

 private:
  BaseClockType::time_point base_time_offset_{};
  time_point scaled_time_offset_{};

  std::vector<ClockableType*> clockables_{};

  void update_clockables(time_point requested_time) noexcept {
    scaled_time_offset_ = current_time();
    base_time_offset_ = BaseClockType::now();
    do {
      time_point override_time{requested_time};
      for (auto* clockable : clockables_) {
        override_time = std::max(scaled_time_offset_,
                                 std::min(override_time, clockable->update_time(override_time)));
      }
      scaled_time_offset_ = override_time;
      for (auto* clockable : clockables_) {
        clockable->run(scaled_time_offset_);
      }
    } while (scaled_time_offset_ != requested_time);
  }

  // Private constructor. Can only be instantiated by the clock() static method.
  ScaledClock() noexcept = default;

 public:
  [[nodiscard]] static time_point now() { return clock().current_time(); }
  [[nodiscard]] static ScaledClock& clock() noexcept {
    static ScaledClock instance{};
    return instance;
  }

  void register_clockable(ClockableType& clockable) noexcept { clockables_.push_back(&clockable); }
  void deregister_clockable(ClockableType& clockable) noexcept {
    std::erase(clockables_, &clockable);
  }

  [[nodiscard]] time_point current_time() noexcept {
    return SCALE_FACTOR * (BaseClockType::now() - base_time_offset_) + scaled_time_offset_;
  }

  // Setters/modifiers for simulation and testing.
  void set_current_time(ScaledClock::time_point t) noexcept { update_clockables(t); }

  void sleep(ScaledClock::time_point t) noexcept { set_current_time(t); }

  template <typename Rep, typename Period>
  void sleep(std::chrono::duration<Rep, Period> d) noexcept {
    set_current_time(current_time() + std::chrono::duration_cast<duration>(d));
  }

  template <typename Rep, typename Period>
  void increment_current_time(std::chrono::duration<Rep, Period> d) noexcept {
    set_current_time(current_time() + std::chrono::duration_cast<duration>(d));
  }

  void increment_current_time_millis(TimeType increment_ms = 1) noexcept {
    set_current_time(current_time() + std::chrono::milliseconds(increment_ms));
  }

  void increment_current_time_micros(TimeType increment_us = 1) noexcept {
    set_current_time(current_time() + std::chrono::microseconds(increment_us));
  }
};

}  // namespace tvsc::hal::time
