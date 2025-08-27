#pragma once

#include "hal/watchdog/watchdog.h"
#include "system/system.h"

namespace tvsc::bringup {

tvsc::system::System::Task run_watchdog(
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
