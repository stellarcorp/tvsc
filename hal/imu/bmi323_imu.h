#pragma once

#include <array>
#include <cstdint>
#include <numbers>

#include "hal/i2c/i2c.h"
#include "hal/imu/imu.h"

namespace tvsc::hal::imu {

class Bmi323Imu final : public ImuPeripheral {
 private:
  static constexpr uint8_t ID_REGISTER{0x00};
  static constexpr uint8_t ERROR_REGISTER{0x01};
  static constexpr uint8_t STATUS_REGISTER{0x02};
  static constexpr uint8_t ACCEL_X_REGISTER{0x03};
  static constexpr uint8_t ACCEL_Y_REGISTER{0x04};
  static constexpr uint8_t ACCEL_Z_REGISTER{0x05};
  static constexpr uint8_t GYRO_X_REGISTER{0x06};
  static constexpr uint8_t GYRO_Y_REGISTER{0x07};
  static constexpr uint8_t GYRO_Z_REGISTER{0x08};
  static constexpr uint8_t TEMP_REGISTER{0x09};

  static constexpr uint8_t ACCEL_CONF_REGISTER{0x20};
  static constexpr uint8_t GYRO_CONF_REGISTER{0x21};

  // Value of LSB in accelerometer reading at each precision configuration. The units are 1 over g.
  static constexpr float ACCEL_LSB_2G{16384};
  static constexpr float ACCEL_LSB_4G{8192};
  static constexpr float ACCEL_LSB_8G{4096};
  static constexpr float ACCEL_LSB_16G{2048};

  // Value of LSB in gyroscope reading at each precision configuration. The units are degrees per
  // second.
  static constexpr float GYRO_LSB_2000{16.384f};
  static constexpr float GYRO_LSB_1000{32.768f};
  static constexpr float GYRO_LSB_500{65.536f};
  static constexpr float GYRO_LSB_250{131.072f};
  static constexpr float GYRO_LSB_125{262.144f};

  // Acceleration due to Earth's gravity in meters per second per second.
  static constexpr float G{9.8067f};

  static constexpr float RADIANS_PER_DEGREE{std::numbers::pi_v<float> / 180.f};

  uint8_t addr_;
  i2c::I2cPeripheral* i2c_peripheral_;
  i2c::I2c i2c_{};

  void enable() override;
  void disable() override;
  bool configure();

  bool read_id(uint16_t& result) override;

  /**
   * Read the accelerometer in units of m/s^2 (meters per second per second), optionally along with
   * the raw register values.
   */
  bool read_accelerometer(std::array<float, 3>& result_mps2,
                          std::array<int16_t, 3>* raw_result) override;

  /**
   * Read the gyroscope in units of rad/s (radians per second), optionally along with the raw
   * register values.
   */
  bool read_gyroscope(std::array<float, 3>& result_radps,
                      std::array<int16_t, 3>* raw_result) override;

  bool put_in_standby_mode() override;

 public:
  Bmi323Imu(uint8_t i2c_addr, i2c::I2cPeripheral& i2c_peripheral)
      : addr_(i2c_addr), i2c_peripheral_(&i2c_peripheral) {}
};

}  // namespace tvsc::hal::imu
