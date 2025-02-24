#include "hal/gpio/gpio_noop.h"

namespace tvsc::hal::gpio {

void GpioNoop::enable() {}

void GpioNoop::disable() {}

void GpioNoop::set_pin_mode(Pin pin, PinMode mode, PinSpeed speed) {}

bool GpioNoop::read_pin(Pin pin) { return false; }

void GpioNoop::write_pin(Pin pin, bool on) {}

void GpioNoop::toggle_pin(Pin pin) {}

}  // namespace tvsc::hal::gpio
