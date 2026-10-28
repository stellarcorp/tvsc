#pragma once

#include <cstdint>

#ifdef __has_include
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

namespace tvsc::hal2 {

#if __cpp_lib_source_location >= 201907L

[[noreturn]] void failure(std::source_location location = std::source_location::current()) noexcept;

#define error() tvsc::hal2::failure()
#define require(expr) ((expr) ? (void)0U : tvsc::hal2::failure())

#else

[[noreturn]] void failure(const char* filename, uint32_t line_number) noexcept;

#define error() tvsc::hal2::failure((__FILE__), (__LINE__))
#define require(expr) ((expr) ? (void)0U : tvsc::hal2::failure((__FILE__), (__LINE__)))

#endif

}  // namespace tvsc::hal2
