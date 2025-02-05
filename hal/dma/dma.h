#pragma once

#include <cstdint>

#include "hal/enable_lock.h"
#include "hal/peripheral_id.h"

namespace tvsc::hal::dma {

class Dma {
 public:
  // Peripheral id for linking peripherals together in a platform-dependent manner.
  virtual PeripheralId id() = 0;

  virtual void start_circular_transfer() = 0;

  virtual void handle_interrupt() = 0;

  // Turn on power and clock to this peripheral.
  virtual EnableLock enable() = 0;
};

}  // namespace tvsc::hal::dma
