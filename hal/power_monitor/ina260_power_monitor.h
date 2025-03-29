#pragma once

#include <cstdint>

#include "hal/i2c/i2c.h"
#include "hal/power_monitor/power_monitor.h"

namespace tvsc::hal::power_monitor {

class Ina260PowerMonitor final : public PowerMonitorPeripheral {
 private:
  static constexpr uint8_t ID_REGISTER{0xff};

  static constexpr uint8_t CONFIGURATION_REGISTER{0x00};
  static constexpr uint8_t CURRENT_REGISTER{0x01};
  static constexpr uint8_t BUS_VOLTAGE_REGISTER{0x02};
  static constexpr uint8_t POWER_REGISTER{0x03};

  uint8_t addr_;
  i2c::I2cPeripheral* i2c_peripheral_;
  i2c::I2c i2c_{};

  void enable() override;
  void disable() override;

  bool shutdown();
  bool configure();

  bool read_id(uint16_t* result) override;
  bool read_current(float* result_amps) override;
  bool read_voltage(float* result_volts) override;
  bool read_power(float* result_watts) override;

 public:
  Ina260PowerMonitor(uint8_t i2c_addr, i2c::I2cPeripheral& i2c_peripheral)
      : addr_(i2c_addr), i2c_peripheral_(&i2c_peripheral) {}
};

}  // namespace tvsc::hal::power_monitor
