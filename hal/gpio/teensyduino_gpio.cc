#include "hal/gpio/teensyduino_gpio.h"

#include <cstdint>

#include "Arduino.h"
#include "hal/gpio/gpio.h"

namespace tvsc::hal::gpio {

void TeensyduinoGpio::set_mode(Pin pin, PinMode mode) {
  pinMode(pin, static_cast<std::underlying_type_t<PinMode>>(mode));
}

bool TeensyduinoGpio::read_pin(Pin pin) { return static_cast<bool>(digitalRead(pin)); }

void TeensyduinoGpio::write_pin(Pin pin, bool value) { digitalWrite(pin, value); }

void TeensyduinoGpio::toggle_pin(Pin pin) { digitalWrite(pin, !digitalRead(pin)); }

}  // namespace tvsc::hal::gpio
