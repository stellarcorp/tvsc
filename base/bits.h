#pragma once

#include <cstdint>

#if __has_cpp_attribute(__cpp_lib_int_pow2)
#include <bit>
#endif

namespace tvsc {

namespace internal {

// If possible, we want to use the std method for computing the bit width, but it is only available
// in C++20.
#if __has_cpp_attribute(__cpp_lib_int_pow2)
inline constexpr std::size_t number_of_bits(std::size_t x) noexcept { return std::bit_width(x); }
#else
inline constexpr std::size_t number_of_bits(std::size_t x) noexcept {
  return x < 2 ? x : 1 + number_of_bits(x >> 1);
}
#endif

}  // namespace internal

/**
 * Compute the number of bits required to store the given value.
 */
inline constexpr uint8_t bit_width(std::size_t value) {
  return static_cast<uint8_t>(internal::number_of_bits(value));
}

}  // namespace tvsc