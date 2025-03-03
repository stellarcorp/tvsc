#pragma once

#include <cstdint>

#ifdef __has_include
#if __has_include(<source_location>)
#include <source_location>
#endif
#endif

namespace tvsc::hal {

#if __cpp_lib_source_location >= 201907L

[[noreturn]] void failure(std::source_location location = std::source_location::current()) noexcept;

#define error() tvsc::hal::failure()
#define require(expr) ((expr) ? (void)0U : tvsc::hal::failure())

#else

[[noreturn]] void failure(const char* filename, uint32_t line_number) noexcept;

#define error() tvsc::hal::failure((__FILE__), (__LINE__))
#define require(expr) ((expr) ? (void)0U : tvsc::hal::failure((__FILE__), (__LINE__)))

#endif

}  // namespace tvsc::hal
