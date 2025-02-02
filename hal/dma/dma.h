#pragma once

#include "hal/power_token.h"

namespace tvsc::hal::dma {

class Dma {
 public:
  virtual void start_circular_transfer() = 0;

  virtual void handle_interrupt() = 0;

  // Turn on power and clock to this peripheral.
  virtual PowerToken enable() = 0;
};

}  // namespace tvsc::hal::dma
