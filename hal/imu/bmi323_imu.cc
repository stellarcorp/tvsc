#include "hal/imu/bmi323_imu.h"

#include <array>
#include <cstdint>

#include "hal/error.h"

namespace tvsc::hal::imu {

void Bmi323Imu::enable() {
  i2c_ = i2c_peripheral_->access();
  require(i2c_.is_device_ready(addr_));

  require(configure());
}

void Bmi323Imu::disable() { i2c_.invalidate(); }

bool Bmi323Imu::configure() {
  // std::array<uint8_t, 4> bytes{};
  // if (!i2c_.read(addr_, ID_REGISTER, bytes_debug.data(), sizeof(bytes_debug))) {
  //   return false;
  // }

  // if (bytes_debug[2] != 0x43) {
  //   require(false);
  //   return false;
  // }

  // if (!i2c_.read(addr_, ERROR_REGISTER, bytes_debug.data(), sizeof(bytes_debug))) {
  //   return false;
  // }

  // // if (bytes_debug[2] != 0) {
  // //   require(false);
  // //   return false;
  // // }

  // // if (bytes_debug[3] != 0) {
  // //   require(false);
  // //   return false;
  // // }

  // if (!i2c_.read(addr_, STATUS_REGISTER, bytes_debug.data(), sizeof(bytes_debug))) {
  //   return false;
  // }

  // // if (bytes_debug[3] != 1 && bytes_debug[3] != 0) {
  // //   require(false);
  // //   return false;
  // // }

  static constexpr uint16_t accelerometer_configuration{0b0'100'0'000'0'010'1000};
  static constexpr uint16_t gyroscope_configuration{0b0'100'0'000'0'010'1000};

  std::array<uint8_t, 2> configuration_bytes{};

  configuration_bytes = {static_cast<uint8_t>(accelerometer_configuration & 0xff),
                         static_cast<uint8_t>(accelerometer_configuration >> 8)};
  if (!i2c_.write(addr_, ACCEL_CONF_REGISTER, configuration_bytes.data(),
                  sizeof(configuration_bytes))) {
    return false;
  }

  configuration_bytes = {static_cast<uint8_t>(gyroscope_configuration & 0xff),
                         static_cast<uint8_t>(gyroscope_configuration >> 8)};
  if (!i2c_.write(addr_, GYRO_CONF_REGISTER, configuration_bytes.data(),
                  sizeof(configuration_bytes))) {
    return false;
  }

  return true;
}

bool Bmi323Imu::put_in_standby_mode() {
  static constexpr uint16_t accelerometer_configuration{0b0'000'0'000'0'010'1000};
  static constexpr uint16_t gyroscope_configuration{0b0'000'0'000'0'010'1000};

  std::array<uint8_t, 2> configuration_bytes{};

  configuration_bytes = {static_cast<uint8_t>(accelerometer_configuration & 0xff),
                         static_cast<uint8_t>(accelerometer_configuration >> 8)};
  if (!i2c_.write(addr_, ACCEL_CONF_REGISTER, configuration_bytes.data(),
                  sizeof(configuration_bytes))) {
    return false;
  }

  configuration_bytes = {static_cast<uint8_t>(gyroscope_configuration & 0xff),
                         static_cast<uint8_t>(gyroscope_configuration >> 8)};
  if (!i2c_.write(addr_, GYRO_CONF_REGISTER, configuration_bytes.data(),
                  sizeof(configuration_bytes))) {
    return false;
  }

  return true;
}

bool Bmi323Imu::read_id(uint16_t& result) {
  uint8_t bytes[4]{};
  bool success{i2c_.read(addr_, ID_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    // Only the 8 LSB of the register value should be kept.
    result = bytes[2];
  }
  return success;
}

/**
 * Read the accelerometer in units of m/s^2 (meters per second per second), optionally along with
 * the raw register values.
 */
bool Bmi323Imu::read_accelerometer(std::array<float, 3>& result_mps2,
                                   std::array<int16_t, 3>* raw_result) {
  std::array<uint8_t, 8> bytes{};
  if (i2c_.read(addr_, ACCEL_X_REGISTER, bytes.data(), sizeof(bytes))) {
    result_mps2[0] = ((static_cast<int16_t>(bytes[3]) << 8) + bytes[2]) / ACCEL_LSB_8G * G;
    result_mps2[1] = ((static_cast<int16_t>(bytes[5]) << 8) + bytes[4]) / ACCEL_LSB_8G * G;
    result_mps2[2] = ((static_cast<int16_t>(bytes[7]) << 8) + bytes[6]) / ACCEL_LSB_8G * G;
    if (raw_result != nullptr) {
      (*raw_result)[0] = (static_cast<int16_t>(bytes[3]) << 8) + bytes[2];
      (*raw_result)[1] = (static_cast<int16_t>(bytes[5]) << 8) + bytes[4];
      (*raw_result)[2] = (static_cast<int16_t>(bytes[7]) << 8) + bytes[6];
    }
  } else {
    return false;
  }

  return true;
}

/**
 * Read the gyroscope in units of rad/s (radians per second), optionally along with the raw
 * register values.
 */
bool Bmi323Imu::read_gyroscope(std::array<float, 3>& result_radps,
                               std::array<int16_t, 3>* raw_result = nullptr) {
  std::array<uint8_t, 8> bytes{};
  if (i2c_.read(addr_, GYRO_X_REGISTER, bytes.data(), sizeof(bytes))) {
    result_radps[0] =
        ((static_cast<int16_t>(bytes[3]) << 8) + bytes[2]) / GYRO_LSB_2000 * RADIANS_PER_DEGREE;
    result_radps[1] =
        ((static_cast<int16_t>(bytes[5]) << 8) + bytes[4]) / GYRO_LSB_2000 * RADIANS_PER_DEGREE;
    result_radps[2] =
        ((static_cast<int16_t>(bytes[7]) << 8) + bytes[6]) / GYRO_LSB_2000 * RADIANS_PER_DEGREE;
    if (raw_result != nullptr) {
      (*raw_result)[0] = (static_cast<int16_t>(bytes[3]) << 8) + bytes[2];
      (*raw_result)[1] = (static_cast<int16_t>(bytes[5]) << 8) + bytes[4];
      (*raw_result)[2] = (static_cast<int16_t>(bytes[7]) << 8) + bytes[6];
    }
  } else {
    return false;
  }

  return true;
}

}  // namespace tvsc::hal::imu
