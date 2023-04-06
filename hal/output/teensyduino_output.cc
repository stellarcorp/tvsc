#include <cstdint>
#include <string_view>

#include "Arduino.h"
#include "hal/output/output.h"

namespace tvsc::hal::output {

void print(std::string_view value) { Serial.print(value.data()); }
void print(uint8_t value) { Serial.print(value); }
void print(uint16_t value) { Serial.print(value); }
void print(uint32_t value) { Serial.print(value); }
void print(uint64_t value) { Serial.print(value); }
void print(int8_t value) { Serial.print(value); }
void print(int16_t value) { Serial.print(value); }
void print(int32_t value) { Serial.print(value); }
void print(int64_t value) { Serial.print(value); }
void print(std::size_t value) { Serial.print(value); }
void print(float value) { Serial.print(value); }
void print(double value) { Serial.print(value); }

void println(std::string_view value) { Serial.println(value.data()); }
void println(uint8_t value) { Serial.println(value); }
void println(uint16_t value) { Serial.println(value); }
void println(uint32_t value) { Serial.println(value); }
void println(uint64_t value) { Serial.println(value); }
void println(int8_t value) { Serial.println(value); }
void println(int16_t value) { Serial.println(value); }
void println(int32_t value) { Serial.println(value); }
void println(int64_t value) { Serial.println(value); }
void println(std::size_t value) { Serial.println(value); }
void println(float value) { Serial.println(value); }
void println(double value) { Serial.println(value); }
void println() { Serial.println(); }

}  // namespace tvsc::hal::output
