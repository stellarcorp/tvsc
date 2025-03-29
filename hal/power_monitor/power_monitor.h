#pragma once

#include <cstdint>

#include "hal/peripheral.h"

namespace tvsc::hal::power_monitor {

class PowerMonitor;

/**
 * Interface to manage sending and receiving messages over an IMU bus.
 */
class PowerMonitorPeripheral : public Peripheral<PowerMonitorPeripheral, PowerMonitor> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual bool read_id(uint16_t* result) = 0;

  friend class PowerMonitor;

 public:
  virtual ~PowerMonitorPeripheral() = default;
};

class PowerMonitor final : public Functional<PowerMonitorPeripheral, PowerMonitor> {
 private:
  PowerMonitor(PowerMonitorPeripheral& peripheral)
      : Functional<PowerMonitorPeripheral, PowerMonitor>(peripheral) {}

  friend class Peripheral<PowerMonitorPeripheral, PowerMonitor>;

 public:
  bool read_id(uint16_t* result) { return peripheral_->read_id(result); }
};

}  // namespace tvsc::hal::power_monitor
