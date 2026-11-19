#pragma once

#include <chrono>

#include "hal2/devices/device.h"

namespace tvsc::hal2::watchdog {

class Watchdog;

class WatchdogDevice : public devices::Device<WatchdogDevice, Watchdog> {
 private:
  // Turn on power and clock to this device.
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void feed() noexcept = 0;

  friend class Watchdog;

 public:
  virtual ~WatchdogDevice() = default;

  [[nodiscard]] virtual std::chrono::milliseconds reset_interval() noexcept = 0;
};

class Watchdog final : public devices::Driver<WatchdogDevice, Watchdog> {
 protected:
  explicit Watchdog(WatchdogDevice& device) noexcept
      : devices::Driver<WatchdogDevice, Watchdog>(device) {}

  friend class devices::Device<WatchdogDevice, Watchdog>;

 public:
  void feed() noexcept { device_->feed(); }

  [[nodiscard]] std::chrono::milliseconds reset_interval() noexcept {
    return device_->reset_interval();
  }
};

}  // namespace tvsc::hal2::watchdog
