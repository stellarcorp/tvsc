#pragma once

#include "hal/peripheral.h"

namespace tvsc::hal::i2c {

/**
 * Interface to manage sending and receiving messages over an I2C bus.
 */
class I2c {
 public:
  virtual ~I2c() = default;

  virtual void send(uint8_t addr, uint8_t* data, uint16_t size, uint32_t timeout) = 0;
  virtual void receive(uint8_t addr, uint8_t* data, uint16_t size, uint32_t timeout) = 0;
};

}  // namespace tvsc::hal::i2c
