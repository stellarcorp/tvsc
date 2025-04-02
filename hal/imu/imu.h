#pragma once

#include <array>
#include <cstdint>

#include "hal/peripheral.h"

namespace tvsc::hal::imu {

class Imu;

/**
 * Interface to manage sending and receiving messages over an IMU bus.
 */
class ImuPeripheral : public Peripheral<ImuPeripheral, Imu> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual bool read_id(uint16_t& result) = 0;

  /**
   * Read the accelerometer in units of m/s^2 (meters per second per second), optionally along with
   * the raw register values.
   */
  virtual bool read_accelerometer(std::array<float, 3>& result_mps2,
                                  std::array<int16_t, 3>* raw_result = nullptr) = 0;

  /**
   * Read the gyroscope in units of rad/s (radians per second), optionally along with the raw
   * register values.
   */
  virtual bool read_gyroscope(std::array<float, 3>& result_radps,
                              std::array<int16_t, 3>* raw_result = nullptr) = 0;

  virtual bool put_in_standby_mode() = 0;

  friend class Imu;

 public:
  virtual ~ImuPeripheral() = default;
};

class Imu final : public Functional<ImuPeripheral, Imu> {
 private:
  Imu(ImuPeripheral& peripheral) : Functional<ImuPeripheral, Imu>(peripheral) {}

  friend class Peripheral<ImuPeripheral, Imu>;

 public:
  bool read_id(uint16_t& result) { return peripheral_->read_id(result); }

  /**
   * Read the accelerometer in units of m/s^2 (meters per second per second), optionally along with
   * the raw register value.
   */
  bool read_accelerometer(std::array<float, 3>& result_mps2,
                          std::array<int16_t, 3>* raw_result = nullptr) {
    return peripheral_->read_accelerometer(result_mps2, raw_result);
  }

  /**
   * Read the gyroscope in units of rad/s (radians per second), optionally along with the raw
   * register value.
   */
  bool read_gyroscope(std::array<float, 3>& result_radps,
                      std::array<int16_t, 3>* raw_result = nullptr) {
    return peripheral_->read_gyroscope(result_radps, raw_result);
  }

  bool put_in_standby_mode() { return peripheral_->put_in_standby_mode(); }
};

}  // namespace tvsc::hal::imu
