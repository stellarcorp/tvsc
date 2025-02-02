#pragma once

#include <cstdint>

#include "hal/enable_lock.h"

namespace tvsc::hal::dac {

class Dac {
 public:
  virtual ~Dac() = default;

  virtual void set_value(uint32_t value, uint8_t channel = 0) = 0;
  virtual void clear_value(uint8_t channel = 0) = 0;

  virtual void set_resolution(uint8_t bits_resolution, uint8_t channel = 0) = 0;

  // Turn on power and clock to this peripheral.
  virtual EnableLock enable() = 0;
};

}  // namespace tvsc::hal::dac
