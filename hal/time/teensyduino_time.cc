#include <cstdint>

#include "Arduino.h"
#include "hal/time/time.h"

namespace tvsc::hal::time {

void delay_ms(uint32_t milliseconds) { delay(milliseconds); }

void delay_us(uint32_t microseconds) { delayMicroseconds(microseconds); }

}  // namespace tvsc::hal::time
