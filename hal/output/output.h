#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace tvsc::hal::output {

// TODO(james): Restructure this API to follow the design of glog so that glog is the most direct
// implementation (or just port glog to Arduino, perhaps basing it on Serial.write()). We can then
// adapt the glog idioms to Arduino's Serial functions.
void print(std::string_view s);
void print(uint8_t value);
void print(uint16_t value);
void print(uint32_t value);
void print(uint64_t value);
void print(int8_t value);
void print(int16_t value);
void print(int32_t value);
void print(int64_t value);
void print(std::size_t value);
void print(float value);
void print(double value);

void println(std::string_view s);
void println(uint8_t value);
void println(uint16_t value);
void println(uint32_t value);
void println(uint64_t value);
void println(int8_t value);
void println(int16_t value);
void println(int32_t value);
void println(int64_t value);
void println(std::size_t value);
void println(float value);
void println(double value);
void println();

}  // namespace tvsc::hal::output
