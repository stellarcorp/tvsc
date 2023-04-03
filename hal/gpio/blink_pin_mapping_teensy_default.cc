#include "Arduino.h"
#include "hal/gpio/blink_pin_mapping.h"

namespace tvsc::hal::gpio {

int BlinkPinMapping::led_pin() { return LED_BUILTIN; }

}  // namespace tvsc::hal::gpio
