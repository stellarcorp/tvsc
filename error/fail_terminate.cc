#include <cstdint>
#include <cstdlib>
#include <exception>

#ifdef __has_include
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

#include "error/fault_message.h"

namespace tvsc::error {

[[noreturn]] void failure(const char* filename, uint32_t line_number) noexcept {
  fault_message.clear();
  fault_message.append(line_number);
  fault_message.append(filename);

  std::terminate();
}

#if __cpp_lib_source_location >= 201907L

[[noreturn]] void failure(std::source_location location) noexcept {
  failure(location.file_name(), location.line());
}

#endif

}  // namespace tvsc::error
