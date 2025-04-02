#pragma once

#include <chrono>
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
  virtual bool read_current(float* result_amps, uint16_t* raw_result = nullptr) = 0;
  virtual bool read_voltage(float* result_volts, uint16_t* raw_result = nullptr) = 0;
  virtual bool read_power(float* result_watts, uint16_t* raw_result = nullptr) = 0;

  virtual bool put_in_standby_mode() = 0;

  friend class PowerMonitor;

 public:
  virtual ~PowerMonitorPeripheral() = default;

  virtual std::chrono::microseconds current_measurement_time() = 0;
  virtual void set_current_measurement_time_approximate(std::chrono::microseconds duration) = 0;

  virtual std::chrono::microseconds voltage_measurement_time() = 0;
  virtual void set_voltage_measurement_time_approximate(std::chrono::microseconds duration) = 0;

  virtual uint16_t sample_averaging() = 0;
  virtual void set_sample_averaging_approximate(uint16_t num_samples) = 0;
};

class PowerMonitor final : public Functional<PowerMonitorPeripheral, PowerMonitor> {
 private:
  PowerMonitor(PowerMonitorPeripheral& peripheral)
      : Functional<PowerMonitorPeripheral, PowerMonitor>(peripheral) {}

  friend class Peripheral<PowerMonitorPeripheral, PowerMonitor>;

 public:
  bool read_id(uint16_t* result) { return peripheral_->read_id(result); }

  bool read_current(float* result_amps, uint16_t* raw_result = nullptr) {
    return peripheral_->read_current(result_amps, raw_result);
  }

  bool read_voltage(float* result_volts, uint16_t* raw_result = nullptr) {
    return peripheral_->read_voltage(result_volts, raw_result);
  }

  bool read_power(float* result_watts, uint16_t* raw_result = nullptr) {
    return peripheral_->read_power(result_watts, raw_result);
  }

  bool put_in_standby_mode() { return peripheral_->put_in_standby_mode(); }
};

}  // namespace tvsc::hal::power_monitor
