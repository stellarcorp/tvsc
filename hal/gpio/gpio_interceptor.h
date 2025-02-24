#pragma once

#include <chrono>

#include "hal/gpio/gpio.h"
#include "hal/interceptor.h"
#include "hal/peripheral.h"

namespace tvsc::hal::gpio {

class GpioInterceptor final : public Interceptor<GpioPeripheral> {
 public:
  GpioInterceptor(GpioPeripheral& gpio) : Interceptor(gpio) {}

  // Turn on power and clock to this peripheral.
  void enable() override;
  void disable() override;

  void set_pin_mode(Pin pin, PinMode mode, PinSpeed speed) override;

  bool read_pin(Pin pin) override;
  void write_pin(Pin pin, bool on) override;
  void toggle_pin(Pin pin) override;
};

}  // namespace tvsc::hal::gpio
