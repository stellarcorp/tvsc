#include "hal/time/mock_clock.h"

#include <chrono>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

MockClock::time_point MockClock::now() noexcept { return clock().current_time(); }

MockClock& MockClock::clock() noexcept {
  static MockClock instance{};
  return instance;
}

void MockClock::update_clockables() noexcept {
  const time_point requested_time{current_time_};
  do {
    time_point override_time{requested_time};
    for (auto* clockable : clockables_) {
      override_time = std::min(override_time, clockable->update_time(override_time));
    }
    current_time_ = override_time;
    for (auto* clockable : clockables_) {
      clockable->run(current_time_);
    }
  } while (current_time_ != requested_time);
}

}  // namespace tvsc::hal::time
