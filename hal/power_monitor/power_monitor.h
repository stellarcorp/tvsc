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
  virtual bool read_current(float* result_amps) = 0;
  virtual bool read_voltage(float* result_volts) = 0;
  virtual bool read_power(float* result_watts) = 0;

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
  bool read_current(float* result_amps) { return peripheral_->read_current(result_amps); }
  bool read_voltage(float* result_volts) { return peripheral_->read_voltage(result_volts); }
  bool read_power(float* result_watts) { return peripheral_->read_power(result_watts); }
};

}  // namespace tvsc::hal::power_monitor
