#pragma once

#include <chrono>
#include <cstdint>

#include "hal/peripheral.h"

namespace tvsc::hal::i2c {

class I2c;

/**
 * Interface to manage sending and receiving messages over an I2C bus.
 */
class I2cPeripheral : public Peripheral<I2cPeripheral, I2c> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual void send(uint8_t addr, const uint8_t* data, uint16_t size,
                    std::chrono::milliseconds timeout) = 0;

  virtual void receive(uint8_t addr, uint8_t* data, uint16_t size,
                       std::chrono::milliseconds timeout) = 0;

  friend class I2c;

 public:
  virtual ~I2cPeripheral() = default;

  virtual void handle_interrupt() = 0;
};

class I2c final : public Functional<I2cPeripheral, I2c> {
 private:
  I2c(I2cPeripheral& peripheral) : Functional<I2cPeripheral, I2c>(peripheral) {}

  friend class Peripheral<I2cPeripheral, I2c>;

 public:
  I2c() = default;

  void send(uint8_t addr, const uint8_t* data, uint16_t size, std::chrono::milliseconds timeout) {
    peripheral_->send(addr, data, size, timeout);
  }

  void receive(uint8_t addr, uint8_t* data, uint16_t size, std::chrono::milliseconds timeout) {
    peripheral_->receive(addr, data, size, timeout);
  }
};

}  // namespace tvsc::hal::i2c
