#include <cstdint>

#include "Arduino.h"
#include "bus/gpio/pins.h"

namespace tvsc::bus::gpio {

void set_mode(uint8_t pin, PinMode mode) {
  pinMode(pin, static_cast<std::underlying_type_t<PinMode>>(mode));
}

DigitalValue read_pin(uint8_t pin) { return static_cast<DigitalValue>(digitalRead(pin)); }

void write_pin(uint8_t pin, DigitalValue value) {
  digitalWrite(pin, static_cast<std::underlying_type_t<DigitalValue>>(value));
}

}  // namespace tvsc::bus::gpio
