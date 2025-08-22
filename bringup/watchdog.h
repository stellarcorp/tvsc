#pragma once

#include "hal/watchdog/watchdog.h"
#include "system/task.h"

namespace tvsc::bringup {

template <typename ClockType>
tvsc::system::Task<ClockType> run_watchdog(
    tvsc::hal::watchdog::WatchdogPeripheral& watchdog_peripheral) {
  const auto feed_interval{watchdog_peripheral.reset_interval() / 4};

  // Enable the watchdog here. After this, if the dog isn't fed on time, the board will reset.
  auto watchdog{watchdog_peripheral.access()};
  while (true) {
    co_yield feed_interval;
    watchdog.feed();
  }
}

}  // namespace tvsc::bringup
