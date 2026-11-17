#include <cstdint>
#include <cstdlib>

#ifdef __has_include
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

#include "error/fault_message.h"
#include "irq/irq.h"

namespace tvsc::error {

[[noreturn]] void failure(const char* filename, uint32_t line_number) noexcept {
  // Disable interrupts so that fewer things have a chance to change memory before we can examine
  // it.
  irq::disable_irq();

  fault_message.clear();
  fault_message.append(line_number);
  fault_message.append(filename);

  while (true) {
    // Just loop until a watchdog forces a reset.
  }
}

#if __cpp_lib_source_location >= 201907L

[[noreturn]] void failure(std::source_location location) noexcept {
  failure(location.file_name(), location.line());
}

#endif

}  // namespace tvsc::error
