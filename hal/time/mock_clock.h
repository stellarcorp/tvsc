#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

class Clockable;

class MockClock final {
 private:
  TimeType current_time_us_{};

  std::vector<Clockable*> clockables_{};

  void update_clockables() noexcept;

  // Private constructor. Can only be instantiated by the clock() static method.
  MockClock() noexcept = default;

 public:
  // C++ Clock types.
  using rep = TimeType;
  using period = std::micro;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<MockClock, duration>;

  // This clock always moves forward and is never adjusted.
  static constexpr bool is_steady{true};

  [[nodiscard]] static time_point now() noexcept;
  [[nodiscard]] static MockClock& clock() noexcept;

  void register_clockable(Clockable& clockable) noexcept { clockables_.push_back(&clockable); }

  [[nodiscard]] TimeType current_time_millis() noexcept;
  [[nodiscard]] TimeType current_time_micros() noexcept;
  [[nodiscard]] time_point current_time() noexcept {
    return time_point{std::chrono::microseconds{current_time_micros()}};
  }

  void sleep_ms(TimeType milliseconds) noexcept;
  void sleep_us(TimeType microseconds) noexcept;

  template <typename Rep, typename Period>
  void sleep(std::chrono::duration<Rep, Period> d) noexcept {
    sleep_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  }
  void sleep(time_point t) noexcept { sleep(t - current_time()); }

  // Setters/modifiers for testing.
  void set_current_time_millis(TimeType current_time_ms) noexcept;
  void increment_current_time_millis(TimeType increment_ms = 1) noexcept;

  void set_current_time_micros(TimeType current_time_us) noexcept;
  void increment_current_time_micros(TimeType increment_us = 1) noexcept;
};

/**
 * A Clockable is a test-only mock that represents a parallel processing subsystem, such as a
 * daughterboard or external chipset. A Clockable registers itself with the MockClock, and then any
 * subsequent calls to set the time result in a call to update the Clockable. The Clockable should
 * then modify its state to reflect the new time.
 */
class Clockable {
 protected:
  MockClock* clock_;

 public:
  Clockable(MockClock& clock) noexcept : clock_(&clock) { clock_->register_clockable(*this); }
  virtual ~Clockable() noexcept = default;

  virtual void update(TimeType current_time_us) noexcept = 0;
};

}  // namespace tvsc::hal::time
