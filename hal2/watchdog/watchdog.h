#pragma once

#include <chrono>

#include "hal2/device.h"

namespace tvsc::hal2::watchdog {

class Watchdog;

class WatchdogDevice : public Device<WatchdogDevice, Watchdog> {
 private:
  // Turn on power and clock to this peripheral.
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void feed() noexcept = 0;

  friend class Watchdog;

 public:
  virtual ~WatchdogDevice() = default;

  [[nodiscard]] virtual std::chrono::milliseconds reset_interval() noexcept = 0;
};

class Watchdog final : public Driver<WatchdogDevice, Watchdog> {
 protected:
  explicit Watchdog(WatchdogDevice& peripheral) noexcept
      : Driver<WatchdogDevice, Watchdog>(peripheral) {}

  friend class Device<WatchdogDevice, Watchdog>;

 public:
  void feed() noexcept { peripheral_->feed(); }

  [[nodiscard]] std::chrono::milliseconds reset_interval() noexcept {
    return peripheral_->reset_interval();
  }
};

}  // namespace tvsc::hal2::watchdog
