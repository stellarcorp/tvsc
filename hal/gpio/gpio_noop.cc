#include "hal/gpio/gpio_noop.h"

#include "hal/gpio/gpio.h"

namespace tvsc::hal::gpio {

void GpioNoop::enable() {}

void GpioNoop::disable() {}

void GpioNoop::set_pin_mode(PinNumber pin, PinMode mode, PinSpeed speed,
                            uint8_t alternate_function_mapping) {}

bool GpioNoop::read_pin(PinNumber pin) { return false; }

void GpioNoop::write_pin(PinNumber pin, bool on) {}

void GpioNoop::toggle_pin(PinNumber pin) {}

PortNumber GpioNoop::port() const { return port_; }

}  // namespace tvsc::hal::gpio
