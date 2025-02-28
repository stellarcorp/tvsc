#include "hal/time/mock_clock.h"

#include <cstdint>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

TimeType MockClock::current_time_micros() noexcept { return current_time_us_; }
TimeType MockClock::current_time_millis() noexcept { return current_time_us_ / 1000; }

void MockClock::set_current_time_millis(TimeType current_time_ms) noexcept {
  current_time_us_ = current_time_ms * 1000;
  update_clockables();
}

void MockClock::increment_current_time_millis(TimeType increment_ms) noexcept {
  current_time_us_ += increment_ms * 1000;
  update_clockables();
}

void MockClock::set_current_time_micros(TimeType current_time_us) noexcept {
  current_time_us_ = current_time_us;
  update_clockables();
}

void MockClock::increment_current_time_micros(TimeType increment_us) noexcept {
  current_time_us_ += increment_us;
  update_clockables();
}

void MockClock::update_clockables() noexcept {
  for (Clockable* clockable : clockables_) {
    clockable->update(current_time_us_);
  }
}

void MockClock::sleep_ms(TimeType t) noexcept { increment_current_time_millis(t); }

void MockClock::sleep_us(TimeType t) noexcept { increment_current_time_micros(t); }

MockClock::time_point MockClock::now() noexcept { return clock().current_time(); }

MockClock& MockClock::clock() noexcept {
  static MockClock instance{};
  return instance;
}

}  // namespace tvsc::hal::time
