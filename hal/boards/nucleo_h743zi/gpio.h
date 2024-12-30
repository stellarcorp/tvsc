#pragma once

#include "hal/gpio/gpio.h"
#include "hal/gpio/pins.h"

extern "C" {
#include "stm32h7xx.h"
}

namespace tvsc::hal::boards::nucleo_h743zi {

class GpioStm32H7xx final : public gpio::Gpio {
 public:
  GpioStm32H7xx(void *base_address) {}

  void set_pin_mode(gpio::Pin pin, gpio::PinMode mode) override {}

  void toggle_pin(gpio::Pin pin) override {}
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
