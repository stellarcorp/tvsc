#include "hal/time/mock_clock.h"

#include <cstdint>

namespace tvsc::hal::time {

uint64_t MockClock::current_time_micros() { return current_time_us_; }
uint64_t MockClock::current_time_millis() { return current_time_us_ / 1000; }

void MockClock::set_current_time_millis(uint64_t current_time_ms) {
  current_time_us_ = current_time_ms * 1000;
  update_clockables();
}

void MockClock::increment_current_time_millis(uint64_t increment_ms) {
  current_time_us_ += increment_ms * 1000;
  update_clockables();
}

void MockClock::set_current_time_micros(uint64_t current_time_us) {
  current_time_us_ = current_time_us;
  update_clockables();
}

void MockClock::increment_current_time_micros(uint64_t increment_us) {
  current_time_us_ += increment_us;
  update_clockables();
}

void MockClock::update_clockables() {
  for (Clockable* clockable : clockables_) {
    clockable->update(current_time_us_);
  }
}

void MockClock::sleep_ms(uint32_t t) { increment_current_time_millis(t); }

void MockClock::sleep_us(uint32_t t) { increment_current_time_micros(t); }

}  // namespace tvsc::hal::time
