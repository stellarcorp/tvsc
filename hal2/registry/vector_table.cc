#include "vector_table.h"

namespace tvsc::hal2::registry {

[[noreturn]] void DefaultInterruptHandler() noexcept {
  while (true) {
    // Loop indefinitely. Do not return.
  }
}

}  // namespace tvsc::hal2::registry
