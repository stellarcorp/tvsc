#pragma once

#include <cstdint>

namespace tvsc::hal::gpio {

/**
 * Type used to identify a GPIO port (specific peripheral) on an processor.
 */
using Port = uint8_t;

/**
 * Type used to identify a Pin on a GPIO port.
 */
using Pin = uint8_t;

enum class PinMode : uint8_t {
  MODE_INPUT = 0,
  MODE_OUTPUT = 1,
  MODE_INPUT_PULLUP = 2,
  MODE_INPUT_PULLDOWN = 3,
};

enum class DigitalValue {
  VALUE_LOW = 0,
  VALUE_HIGH = 1,
};

void initialize_gpio();

void set_mode(uint8_t pin, PinMode mode);
DigitalValue read_pin(uint8_t pin);
void write_pin(uint8_t pin, DigitalValue value);

}  // namespace tvsc::hal::gpio
