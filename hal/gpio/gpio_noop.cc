#include "hal/gpio/gpio_noop.h"

namespace tvsc::hal::gpio {

void GpioNoop::enable() {}

void GpioNoop::disable() {}

void GpioNoop::set_pin_mode(Pin pin, PinMode mode, PinSpeed speed,
                            uint8_t alternate_function_mapping) {}

bool GpioNoop::read_pin(Pin pin) { return false; }

void GpioNoop::write_pin(Pin pin, bool on) {}

void GpioNoop::toggle_pin(Pin pin) {}

Port GpioNoop::port() const { return port_; }

}  // namespace tvsc::hal::gpio
