#include <cstdint>
#include <iostream>
#include <string_view>

#include "hal/output/output.h"

namespace tvsc::hal::output {

void print(std::string_view value) { std::cout << value; }
void print(uint8_t value) { std::cout << value; }
void print(uint16_t value) { std::cout << value; }
void print(uint32_t value) { std::cout << value; }
void print(uint64_t value) { std::cout << value; }
void print(int8_t value) { std::cout << value; }
void print(int16_t value) { std::cout << value; }
void print(int32_t value) { std::cout << value; }
void print(int64_t value) { std::cout << value; }
void print(float value) { std::cout << value; }
void print(double value) { std::cout << value; }

void println(std::string_view value) { std::cout << value << "\n"; }
void println(uint8_t value) { std::cout << value << "\n"; }
void println(uint16_t value) { std::cout << value << "\n"; }
void println(uint32_t value) { std::cout << value << "\n"; }
void println(uint64_t value) { std::cout << value << "\n"; }
void println(int8_t value) { std::cout << value << "\n"; }
void println(int16_t value) { std::cout << value << "\n"; }
void println(int32_t value) { std::cout << value << "\n"; }
void println(int64_t value) { std::cout << value << "\n"; }
void println(float value) { std::cout << value << "\n"; }
void println(double value) { std::cout << value << "\n"; }
void println() { std::cout << "\n"; }

}  // namespace tvsc::hal::output
