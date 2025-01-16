#pragma once

#include "hal/gpio/gpio.h"

namespace tvsc::hal::rcc {

/**
 * Interface to manage the reset and clock circuitry (RCC) as well as power for select peripherals.
 */
class Rcc {
 public:
  virtual ~Rcc() = default;

  virtual void enable_gpio_port(gpio::Port port) = 0;
  virtual void disable_gpio_port(gpio::Port port) = 0;
};

}  // namespace tvsc::hal::rcc
