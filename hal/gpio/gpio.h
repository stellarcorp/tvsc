#pragma once

#include <cstdint>

namespace tvsc::hal::gpio {

using Pin = uint8_t;

class Gpio {
 public:
  virtual ~Gpio() = default;

  virtual void toggle_pin(Pin pin) = 0;
};

}  // namespace tvsc::hal::gpio
