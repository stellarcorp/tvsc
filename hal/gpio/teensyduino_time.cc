#include <cstdint>

#include "Arduino.h"
#include "bus/gpio/time.h"

namespace tvsc::bus::gpio {

void delay_ms(uint32_t milliseconds) { delay(milliseconds); }

void delay_us(uint32_t microseconds) { delayMicroseconds(microseconds); }

}  // namespace tvsc::bus::gpio
