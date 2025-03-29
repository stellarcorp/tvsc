#pragma once

#include <chrono>
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

  virtual bool read_id(uint16_t* result) = 0;

  friend class Imu;

 public:
  virtual ~ImuPeripheral() = default;
};

class Imu final : public Functional<ImuPeripheral, Imu> {
 private:
  Imu(ImuPeripheral& peripheral) : Functional<ImuPeripheral, Imu>(peripheral) {}

  friend class Peripheral<ImuPeripheral, Imu>;

 public:
  bool read_id(uint16_t* result) { return peripheral_->read_id(result); }
};

}  // namespace tvsc::hal::imu
