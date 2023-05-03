#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace tvsc::hash {

namespace internal {

template <uint8_t bits>
uint64_t rotate(uint64_t value) {
  return (value >> bits) | (value << (64 - bits));
}

template <uint8_t bits>
uint32_t rotate(uint32_t value) {
  return (value >> bits) | (value << (32 - bits));
}

inline std::size_t integer_hash_64(uint64_t value) {
  // rrxmrrxmsx_0 from Pelle Evensen
  // http://mostlymangling.blogspot.com/2019/01/better-stronger-mixer-and-test-procedure.html
  value ^= rotate<25>(value) ^ rotate<50>(value);
  value *= 0xA24BAED4963EE407UL;
  value ^= rotate<24>(value) ^ rotate<49>(value);
  value *= 0x9FB21C651E98DF25UL;
  return value ^ (value >> 28);
}

inline std::size_t integer_hash_32(uint32_t value) {
  // Based on similar approach as integer_hash_64(), but with values adapted for 32-bit integers.
  value ^= rotate<12>(value) ^ rotate<24>(value);
  value *= 0x963EE407U;
  value ^= rotate<11>(value) ^ rotate<23>(value);
  value *= 0x1E98DF25U;
  return value ^ (value >> 14);
}

}  // namespace internal

/**
 * Hash functions for integers that have low intrinsic entropy, such as a counter. The default C++
 * hash function for integers (usually) just returns the input itself. That implementation is good
 * if the possible inputs are well-distributed across the integers. But if the values are mostly
 * small, especially if they are simple sequences, that implementation tends to yield many hash
 * collisions.
 *
 * These implementations are slower than just returning the input but provide better mixing of bits
 * in most cases.
 */
template <typename IntegerT, std::enable_if_t<std::is_integral<IntegerT>::value, bool> = true>
std::size_t integer_hash(IntegerT value) {
  if constexpr (std::is_same_v<std::size_t, uint32_t>) {
    return internal::integer_hash_32(static_cast<uint32_t>(value));
  } else if constexpr (std::is_same_v<std::size_t, uint64_t>) {
    return internal::integer_hash_64(static_cast<uint64_t>(value));
  } else if constexpr (sizeof(std::size_t) == sizeof(uint32_t)) {
    // Some platforms seem to implement std::size_t as the same size as uint32_t but the two are not
    // the same type. Not sure why this is done. We leave the is_same<> checks in place for
    // documentation, even though these sizeof() checks would be sufficient.
    return internal::integer_hash_32(static_cast<uint32_t>(value));
  } else if constexpr (sizeof(std::size_t) == sizeof(uint64_t)) {
    // See sizeof(uint32_t) case comment.
    return internal::integer_hash_64(static_cast<uint64_t>(value));
  } else {
    static_assert(std::is_same_v<std::size_t, uint32_t> ||  //
                      std::is_same_v<std::size_t, uint64_t> ||
                      sizeof(std::size_t) == sizeof(uint32_t) ||
                      sizeof(std::size_t) == sizeof(uint64_t),
                  "Can't understand how std::size_t is implemented.");
  }
}

/**
 * Always use a 64-bit hash function for 64-bit values.
 */
template <>
inline std::size_t integer_hash<uint64_t>(uint64_t value) {
  return internal::integer_hash_64(value);
}

}  // namespace tvsc::hash
