#include "hal/watchdog/watchdog_noop.h"

#include <chrono>

namespace tvsc::hal::watchdog {

void WatchdogNoop::enable() {}

void WatchdogNoop::disable() {}

void WatchdogNoop::feed() {}

[[nodiscard]] std::chrono::milliseconds WatchdogNoop::reset_interval() {
  using namespace std::chrono_literals;
  return 1s;
}

}  // namespace tvsc::hal::watchdog
