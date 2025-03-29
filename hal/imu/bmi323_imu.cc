#include "hal/imu/bmi323_imu.h"

#include "hal/error.h"

namespace tvsc::hal::imu {

void Bmi323Imu::enable() {
  i2c_ = i2c_peripheral_->access();
  require(i2c_.is_device_ready(addr_));
}

void Bmi323Imu::disable() { i2c_.invalidate(); }

bool Bmi323Imu::read_id(uint16_t* result) {
  uint8_t bytes[4]{};
  bool success{i2c_.read(addr_, ID_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    // Only the 8 LSB of the register value should be kept.
    *result = bytes[2];
  }
  return success;
}

}  // namespace tvsc::hal::imu
