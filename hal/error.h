#pragma once

#include <cstdint>

namespace tvsc::hal {

[[noreturn]] void failure(const char* filename, uint32_t line_number) noexcept;

#define error() failure(__FILE__, __LINE__)
#define require(expr) ((expr) ? (void)0U : failure(__FILE__, __LINE__))

}  // namespace tvsc::hal
