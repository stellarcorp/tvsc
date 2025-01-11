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

/**
 * Deprecated. To be removed soon.
 */
enum class PinModeOld : uint8_t {
  MODE_INPUT = 0,
  MODE_OUTPUT = 1,
  MODE_INPUT_PULLUP = 2,
  MODE_INPUT_PULLDOWN = 3,
};

/**
 * Union across all relevant platforms of all possible modes for a pin. Some modes will not be
 * supported on some platforms.
 */
enum class PinMode : uint8_t {
  // Pin is unused and should be put in the state that uses the least amount of power. For most
  // pins (typically any pin not involved in debug), this is likely to be the reset state.
  UNUSED,

  // Analog input or output. Pull-up and pull-down resistors and MOSFETs controlled through
  // functions managing analog resolution, voltage reference, etc.
  ANALOG,

  // Digital input with pull-up and pull-down resistors disabled.
  INPUT_FLOATING,

  // Digital input with pull-up resistor enabled and pull-down resistor disabled.
  INPUT_WITH_PULL_UP,

  // Digital input with pull-up resistor disabled and pull-down resistor enabled.
  INPUT_WITH_PULL_DOWN,

  // Digital output with MOSFETs in a push-pull configuration. Pull-up and pull-down resistors
  // disabled.
  OUTPUT_PUSH_PULL,

  // Digital output with MOSFETs in a push-pull configuration. Pull-up resistor enabled. Pull-down
  // resistor disabled.
  OUTPUT_PUSH_PULL_WITH_PULL_UP,

  // Digital output with MOSFETs in a push-pull configuration. Pull-up resistor disabled. Pull-down
  // resistor enabled.
  OUTPUT_PUSH_PULL_WITH_PULL_DOWN,

  // Digital output with MOSFETs in an open drain configuration. Pull-up and pull-down resistors
  // disabled.
  OUTPUT_OPEN_DRAIN,

  // Digital output with MOSFETs in an open drain configuration. Pull-up resistor enabled. Pull-down
  // resistor disabled.
  OUTPUT_OPEN_DRAIN_WITH_PULL_UP,

  // Digital output with MOSFETs in an open drain configuration. Pull-up resistor disabled.
  // Pull-down resistor enabled.
  OUTPUT_OPEN_DRAIN_WITH_PULL_DOWN,

  // Alternate functions. Using these modes allows the pin to be used by another peripheral and not
  // as a general purpose IO pin.

  // Pin set to alternate function with MOSFETs in a push-pull configuration. Pull-up and pull-down
  // resistors disabled.
  ALTERNATE_FUNCTION_PUSH_PULL,

  // Pin set to alternate function with MOSFETs in a push-pull configuration. Pull-up resistor
  // enabled. Pull-down resistor disabled.
  ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_UP,

  // Pin set to alternate function with MOSFETs in a push-pull configuration. Pull-up resistor
  // disabled. Pull-down resistor enabled.
  ALTERNATE_FUNCTION_PUSH_PULL_WITH_PULL_DOWN,

  // Pin set to alternate function with MOSFETs in an open drain configuration. Pull-up and
  // pull-down resistors disabled.
  ALTERNATE_FUNCTION_OPEN_DRAIN,

  // Pin set to alternate function with MOSFETs in an open drain configuration. Pull-up resistor
  // enabled. Pull-down resistor disabled.
  ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_UP,

  // Pin set to alternate function with MOSFETs in an open drain configuration. Pull-up resistor
  // disabled. Pull-down resistor enabled.
  ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_DOWN,
};

enum class PinSpeed : uint8_t {
  LOW,
  MEDIUM,
  HIGH,
  VERY_HIGH,
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
