#pragma once

#include <chrono>
#include <vector>

#include "hal/time/clock.h"
#include "hal/time/clockable.h"

namespace tvsc::hal::time {

class MockClock final {
 public:
  using ClockableType = Clockable<MockClock>;

  // C++ Clock types.
  using rep = TimeType;
  using period = std::micro;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<MockClock, duration>;

  // This clock always moves forward and is never adjusted.
  static constexpr bool is_steady{true};

 private:
  time_point current_time_{};

  std::vector<ClockableType*> clockables_{};

  void update_clockables() noexcept;

  // Private constructor. Can only be instantiated by the clock() static method.
  MockClock() noexcept = default;

 public:
  [[nodiscard]] static time_point now() noexcept;
  [[nodiscard]] static MockClock& clock() noexcept;

  void register_clockable(ClockableType& clockable) noexcept { clockables_.push_back(&clockable); }

  [[nodiscard]] time_point current_time() noexcept { return current_time_; }

  void sleep(MockClock::time_point t) noexcept { set_current_time(t); }

  template <typename Rep, typename Period>
  void sleep(std::chrono::duration<Rep, Period> d) noexcept {
    set_current_time(current_time_ + std::chrono::duration_cast<duration>(d));
  }

  // Setters/modifiers for testing.
  void set_current_time(MockClock::time_point t) noexcept {
    current_time_ = t;
    update_clockables();
  }

  template <typename Rep, typename Period>
  void increment_current_time(std::chrono::duration<Rep, Period> d) noexcept {
    set_current_time(current_time_ + std::chrono::duration_cast<duration>(d));
  }

  void increment_current_time_millis(TimeType increment_ms = 1) noexcept {
    set_current_time(current_time_ + std::chrono::milliseconds(increment_ms));
  }
  void increment_current_time_micros(TimeType increment_us = 1) noexcept {
    set_current_time(current_time_ + std::chrono::microseconds(increment_us));
  }
};

}  // namespace tvsc::hal::time
