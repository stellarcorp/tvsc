#pragma once

#include <chrono>

#include "hal/peripheral.h"

namespace tvsc::hal::watchdog {

class Watchdog;

class WatchdogPeripheral : public Peripheral<WatchdogPeripheral, Watchdog> {
 private:
  // Turn on power and clock to this peripheral.
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual void feed() = 0;

  friend class Watchdog;

 public:
  virtual ~WatchdogPeripheral() = default;

  [[nodiscard]] virtual std::chrono::milliseconds reset_interval() = 0;
};

class Watchdog final : public Functional<WatchdogPeripheral, Watchdog> {
 protected:
  explicit Watchdog(WatchdogPeripheral& peripheral)
      : Functional<WatchdogPeripheral, Watchdog>(peripheral) {}

  friend class Peripheral<WatchdogPeripheral, Watchdog>;

 public:
  void feed() { peripheral_->feed(); }

  [[nodiscard]] std::chrono::milliseconds reset_interval() { return peripheral_->reset_interval(); }
};

}  // namespace tvsc::hal::watchdog
