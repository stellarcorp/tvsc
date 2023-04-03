#include <cstdint>

#include "Arduino.h"
#include "hal/gpio/time.h"

namespace tvsc::hal::gpio {

void delay_ms(uint32_t milliseconds) { delay(milliseconds); }

void delay_us(uint32_t microseconds) { delayMicroseconds(microseconds); }

}  // namespace tvsc::hal::gpio
