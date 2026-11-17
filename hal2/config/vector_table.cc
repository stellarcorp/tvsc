#include "vector_table.h"

namespace tvsc::hal2::config {

[[noreturn]] void DefaultInterruptHandler() noexcept {
  while (true) {
    // Loop indefinitely. Do not return.
  }
}

}  // namespace tvsc::hal2::config
