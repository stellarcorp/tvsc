#pragma once

#include <chrono>

#include "hal/peripheral.h"
#include "hal/simulation/interceptor.h"
#include "hal/simulation/logger.h"
#include "hal/watchdog/watchdog.h"

namespace tvsc::hal::watchdog {

template <typename ClockType>
class WatchdogInterceptor final : public simulation::Interceptor<WatchdogPeripheral, ClockType> {
 public:
  WatchdogInterceptor(WatchdogPeripheral& watchdog, simulation::Logger<ClockType>& logger)
      : simulation::Interceptor<WatchdogPeripheral, ClockType>(watchdog, logger) {}

  void enable() override {
    LOG_FN();
    return this->call(&WatchdogPeripheral::enable);
  }

  void disable() override {
    LOG_FN();
    return this->call(&WatchdogPeripheral::disable);
  }

  void feed() override {
    LOG_FN();
    return this->call(&WatchdogPeripheral::feed);
  }

  [[nodiscard]] std::chrono::milliseconds reset_interval() override {
    LOG_FN();
    return this->call(&WatchdogPeripheral::reset_interval);
  }
};

}  // namespace tvsc::hal::watchdog
