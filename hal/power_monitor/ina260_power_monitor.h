#pragma once

#include <cstdint>

#include "hal/i2c/i2c.h"
#include "hal/power_monitor/power_monitor.h"

namespace tvsc::hal::power_monitor {

class Ina260PowerMonitor final : public PowerMonitorPeripheral {
 private:
  static constexpr uint8_t ID_REGISTER{0xff};

  uint8_t addr_;
  i2c::I2cPeripheral* i2c_peripheral_;
  i2c::I2c i2c_{};

  void enable() override;
  void disable() override;

  bool read_id(uint16_t* result) override;

 public:
  Ina260PowerMonitor(uint8_t i2c_addr, i2c::I2cPeripheral& i2c_peripheral)
      : addr_(i2c_addr), i2c_peripheral_(&i2c_peripheral) {}
};

}  // namespace tvsc::hal::power_monitor
