#pragma once

#include <cstdint>
#include <cstdlib>

#include "hal/enable_lock.h"
#include "hal/gpio/gpio.h"
#include "hal/peripheral_id.h"

namespace tvsc::hal::timer {

class Timer {
 public:
  virtual ~Timer() = default;

  // Peripheral id for linking peripherals together in a platform-dependent manner.
  virtual PeripheralId id() = 0;

  virtual void start(uint32_t interval_us, bool high_precision = false) = 0;
  virtual void stop() = 0;

  virtual bool is_running() = 0;

  // Turn on power and clock to this peripheral.
  virtual EnableLock enable() = 0;

  virtual void handle_interrupt() = 0;
};

}  // namespace tvsc::hal::timer
