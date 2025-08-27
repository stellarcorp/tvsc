#pragma once

#include "hal/gpio/gpio.h"

namespace tvsc::hal::gpio {

class GpioNoop final : public GpioPeripheral {
 private:
  PortNumber port_;

 public:
  GpioNoop(PortNumber port) : port_(port) {}

  void enable() override;
  void disable() override;

  void set_pin_mode(PinNumber pin, PinMode mode, PinSpeed speed,
                    uint8_t alternate_function_mapping) override;

  bool read_pin(PinNumber pin) override;
  void write_pin(PinNumber pin, bool on) override;
  void toggle_pin(PinNumber pin) override;

  PortNumber port() const override;
};

}  // namespace tvsc::hal::gpio
