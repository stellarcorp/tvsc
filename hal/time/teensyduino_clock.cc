#include "hal/time/teensyduino_clock.h"

#include "Arduino.h"
#include "hal/time/clock.h"

namespace tvsc::hal::time {

TimeType TeensyduinoClock::current_time_millis() { return millis(); }
TimeType TeensyduinoClock::current_time_micros() { return micros(); }

void TeensyduinoClock::sleep_ms(TimeType milliseconds) { delay(milliseconds); }
void TeensyduinoClock::sleep_us(TimeType microseconds) { delayMicroseconds(microseconds); }

}  // namespace tvsc::hal::time
