#pragma once

#include <cstdint>

#include "hal/enable_lock.h"

namespace tvsc::hal::random {

class Rng {
 public:
  virtual ~Rng() = default;

  // Turn on power and clock to this peripheral.
  virtual EnableLock enable() = 0;

  virtual uint32_t operator()() = 0;
};

}  // namespace tvsc::hal::random
