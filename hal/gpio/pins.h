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

/**
 * Deprecated. To be removed soon.
 */
enum class DigitalValue {
  VALUE_LOW = 0,
  VALUE_HIGH = 1,
};

/**
 * Deprecated. To be removed soon.
 */
void initialize_gpio();

/**
 * Deprecated. To be removed soon.
 */
void set_mode(uint8_t pin, PinMode mode);

/**
 * Deprecated. To be removed soon.
 */
DigitalValue read_pin(uint8_t pin);

/**
 * Deprecated. To be removed soon.
 */
void write_pin(uint8_t pin, DigitalValue value);

}  // namespace tvsc::hal::gpio
