#include "time/remote_clock.h"

#include <cstdint>

namespace tvsc::time {

TimeType RemoteClock::current_time_micros() {
  double time_us = local_clock_->current_time_micros();
  time_us += skew_us_;
  return static_cast<TimeType>(time_us);
}

TimeType RemoteClock::current_time_millis() { return current_time_micros() / 1000; }

void RemoteClock::sleep_ms(TimeType t) { local_clock_->sleep_ms(t); }

void RemoteClock::sleep_us(TimeType t) { local_clock_->sleep_us(t); }

void RemoteClock::mark_remote_time_micros(TimeType remote_time_us) {
  double local_time_us = local_clock_->current_time_micros();
  skew_us_ = remote_time_us - local_time_us;
}

}  // namespace tvsc::time
