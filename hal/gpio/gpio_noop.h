#pragma once

#include <chrono>

#include "hal/gpio/gpio.h"
#include "hal/peripheral.h"

namespace tvsc::hal::gpio {

class GpioNoop final : public GpioPeripheral {
 public:
  void enable() override;
  void disable() override;

  void set_pin_mode(Pin pin, PinMode mode, PinSpeed speed,
                    uint8_t alternate_function_mapping) override;

  bool read_pin(Pin pin) override;
  void write_pin(Pin pin, bool on) override;
  void toggle_pin(Pin pin) override;
};

}  // namespace tvsc::hal::gpio
