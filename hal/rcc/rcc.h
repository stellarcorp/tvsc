#pragma once

#include <cstdint>

#include "hal/gpio/gpio.h"

namespace tvsc::hal::rcc {

/**
 * Interface to manage the reset and clock circuitry (RCC) as well as power for select peripherals.
 */
class Rcc {
 public:
  virtual ~Rcc() = default;

  virtual void set_clock_to_max_speed() = 0;
  virtual void set_clock_to_min_speed() = 0;
};

}  // namespace tvsc::hal::rcc
