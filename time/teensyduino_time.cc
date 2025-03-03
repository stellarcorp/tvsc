#include <cstdint>

#include "Arduino.h"
#include "time/time.h"

namespace tvsc::time {

void delay_ms(TimeType milliseconds) { delay(milliseconds); }

void delay_us(TimeType microseconds) { delayMicroseconds(microseconds); }

TimeType time_millis() { return millis(); }

TimeType time_micros() { return micros(); }

}  // namespace tvsc::time
