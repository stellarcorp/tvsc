#pragma once

#include <chrono>

#include "hal/interceptor.h"
#include "hal/peripheral.h"
#include "hal/watchdog/watchdog.h"

namespace tvsc::hal::watchdog {

class WatchdogNoop final : public WatchdogPeripheral {
 public:
  void enable() override;
  void disable() override;

  void feed() override;

  [[nodiscard]] std::chrono::milliseconds reset_interval() override;
};

}  // namespace tvsc::hal::watchdog
