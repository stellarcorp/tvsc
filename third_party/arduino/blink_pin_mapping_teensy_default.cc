#include "Arduino.h"
#include "third_party/arduino/blink_pin_mapping.h"

namespace tvsc::third_party::arduino {

int BlinkPinMapping::led_pin() { return LED_BUILTIN; }

}  // namespace tvsc::third_party::arduino
