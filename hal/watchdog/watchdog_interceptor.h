#pragma once

#include <chrono>

#include "hal/interceptor.h"
#include "hal/peripheral.h"
#include "hal/watchdog/watchdog.h"

namespace tvsc::hal::watchdog {

class WatchdogInterceptor final : public Interceptor<WatchdogPeripheral> {
 public:
  WatchdogInterceptor(WatchdogPeripheral& watchdog) : Interceptor(watchdog) {}

  // Turn on power and clock to this peripheral.
  void enable() override;
  void disable() override;

  void feed() override;

  [[nodiscard]] std::chrono::milliseconds reset_interval() override;
};

}  // namespace tvsc::hal::watchdog
