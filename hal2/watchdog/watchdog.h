#pragma once

#include <chrono>

#include "hal2/peripheral.h"

namespace tvsc::hal2::watchdog {

class Watchdog;

class WatchdogPeripheral : public Peripheral<WatchdogPeripheral, Watchdog> {
 private:
  // Turn on power and clock to this peripheral.
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void feed() noexcept = 0;

  friend class Watchdog;

 public:
  virtual ~WatchdogPeripheral() = default;

  [[nodiscard]] virtual std::chrono::milliseconds reset_interval() noexcept = 0;
};

class Watchdog final : public Driver<WatchdogPeripheral, Watchdog> {
 protected:
  explicit Watchdog(WatchdogPeripheral& peripheral) noexcept
      : Driver<WatchdogPeripheral, Watchdog>(peripheral) {}

  friend class Peripheral<WatchdogPeripheral, Watchdog>;

 public:
  void feed() noexcept { peripheral_->feed(); }

  [[nodiscard]] std::chrono::milliseconds reset_interval() noexcept {
    return peripheral_->reset_interval();
  }
};

}  // namespace tvsc::hal2::watchdog
