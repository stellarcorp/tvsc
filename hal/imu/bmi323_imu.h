#pragma once

#include <cstdint>

#include "hal/i2c/i2c.h"
#include "hal/imu/imu.h"

namespace tvsc::hal::imu {

class Bmi323Imu final : public ImuPeripheral {
 private:
  static constexpr uint8_t ID_REGISTER{0x00};

  uint8_t addr_;
  i2c::I2cPeripheral* i2c_peripheral_;
  i2c::I2c i2c_{};

  void enable() override;
  void disable() override;

  bool read_id(uint16_t* result) override;

 public:
  Bmi323Imu(uint8_t i2c_addr, i2c::I2cPeripheral& i2c_peripheral)
      : addr_(i2c_addr), i2c_peripheral_(&i2c_peripheral) {}
};

}  // namespace tvsc::hal::imu
