#pragma once

#include <cstdint>

#include "hal/gpio/pins.h"

namespace tvsc::hal::gpio {

class Gpio {
 public:
  virtual ~Gpio() = default;

  virtual void set_pin_mode(Pin pin, PinMode mode) = 0;

  virtual void toggle_pin(Pin pin) = 0;
};

}  // namespace tvsc::hal::gpio
