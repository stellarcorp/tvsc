#pragma once

#include <cstdint>

#include "hal/gpio/gpio.h"

namespace tvsc::hal::gpio {

class TeensyduinoGpio final : public Gpio {
  void set_mode(Pin pin, PinMode mode) override;

  bool read_pin(Pin pin) override;

  void write_pin(Pin pin, bool value) override;

  void toggle_pin(Pin pin) override;
};

}  // namespace tvsc::hal::gpio
