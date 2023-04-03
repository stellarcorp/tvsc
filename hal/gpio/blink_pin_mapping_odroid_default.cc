#include "hal/gpio/blink_pin_mapping.h"

namespace tvsc::hal::gpio {

// Note: here we are using the Broadcom GPIO pin numbers directly. We assume that these mappings
// will be used with WiringPi's wiringPiSetupGpio().

int BlinkPinMapping::led_pin() { return 91; }

}  // namespace tvsc::hal::gpio
