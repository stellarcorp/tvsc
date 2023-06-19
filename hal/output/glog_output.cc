#include <cstdint>
#include <string_view>

#include "glog/logging.h"
#include "hal/output/output.h"

namespace tvsc::hal::output {

void print(std::string_view value) { LOG(INFO) << value; }
void print(uint8_t value) { LOG(INFO) << value; }
void print(uint16_t value) { LOG(INFO) << value; }
void print(uint32_t value) { LOG(INFO) << value; }
void print(uint64_t value) { LOG(INFO) << value; }
void print(int8_t value) { LOG(INFO) << value; }
void print(int16_t value) { LOG(INFO) << value; }
void print(int32_t value) { LOG(INFO) << value; }
void print(int64_t value) { LOG(INFO) << value; }
void print(float value) { LOG(INFO) << value; }
void print(double value) { LOG(INFO) << value; }

void println(std::string_view value) { LOG(INFO) << value << "\n"; }
void println(uint8_t value) { LOG(INFO) << value << "\n"; }
void println(uint16_t value) { LOG(INFO) << value << "\n"; }
void println(uint32_t value) { LOG(INFO) << value << "\n"; }
void println(uint64_t value) { LOG(INFO) << value << "\n"; }
void println(int8_t value) { LOG(INFO) << value << "\n"; }
void println(int16_t value) { LOG(INFO) << value << "\n"; }
void println(int32_t value) { LOG(INFO) << value << "\n"; }
void println(int64_t value) { LOG(INFO) << value << "\n"; }
void println(float value) { LOG(INFO) << value << "\n"; }
void println(double value) { LOG(INFO) << value << "\n"; }
void println() { LOG(INFO) << "\n"; }

}  // namespace tvsc::hal::output
