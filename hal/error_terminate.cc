#include <cstdint>
#include <cstdlib>
#include <exception>

#include "hal/irq.h"

namespace tvsc::hal {

struct ErrorLocation {
  uint32_t line_number{};
  // Note that this sizing is intentional. The system has failed. We are about to reset. This
  // filename gives us debug information that might help. But, we don't want to allocate some large
  // buffer to properly hold the filename, since that would restrict using that RAM for another
  // purpose. That is, we choose to blast the filename over everything else in RAM, rather than
  // restrict that RAM from being used in normal operations. So, we are now "wasting" four bytes, as
  // the section is word-aligned, instead of whatever would be required to properly hold any
  // filename possible. To that end, we specifically place this filename at the end of the status
  // section (see the linker scripts for details) so that we preserve anything in the status
  // section.
  char filename[1];
};

__attribute__((section(".status.fault"))) ErrorLocation error_location{};

[[noreturn]] void failure(const char* filename, uint32_t line_number) noexcept {
  static constexpr size_t LARGEST_FILENAME_ALLOWED{4096};

  // Disable interrupts so that fewer things have a chance to change memory before we can examine
  // it.
  disable_irq();

  error_location.line_number = line_number;

  // Note that we specifically do NOT use strncpy() here or some variant. We want to save that
  // filename and definitely did not allocate memory for it. To that end, most strncpy() variants
  // indicate undefined behavior if we go beyond the allocated size of the destination buffer. While
  // we are pretty certain that means it will just corrupt memory past the buffer, we would rather
  // do that ourselves than risk some unforeseen, truly undefined behavior. (Note: "undefined
  // behavior" means the compiler can generate anything it wants, including no code.) Also, see
  // comment in ErrorLocation.
  for (size_t i = 0; i < LARGEST_FILENAME_ALLOWED; ++i) {
    char c = filename[i];
    error_location.filename[i] = c;
    if (c == '\0') {
      break;
    }
  }

  std::terminate();
}

}  // namespace tvsc::hal
