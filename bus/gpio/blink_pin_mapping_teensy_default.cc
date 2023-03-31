#include "Arduino.h"
#include "bus/gpio/blink_pin_mapping.h"

namespace tvsc::bus::gpio {

int BlinkPinMapping::led_pin() { return LED_BUILTIN; }

}  // namespace tvsc::bus::gpio
