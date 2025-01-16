#pragma once

#include <cstdint>

#ifdef __has_include
#if __has_include(<bit>)
#include <bit>
#endif
#endif

namespace tvsc {

namespace internal {

// If possible, we want to use the std method for computing the bit width, but it is only available
// in C++20.
#if __cpp_lib_int_pow2 >= 202002L
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

template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET>
inline constexpr uint32_t compute_bit_mask() {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= 32,
                "Invalid mask parameters. The total number of bits (offset and the number "
                "of bits in the mask) must be less than or equal to the size of the word "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  return ((1UL << NUM_BITS) - 1UL) << BIT_FIELD_OFFSET;
}

template <uint8_t NUM_BITS>
inline constexpr uint32_t compute_bit_mask(uint8_t bit_field_offset) {
  return ((1UL << NUM_BITS) - 1UL) << bit_field_offset;
}

inline constexpr uint32_t compute_bit_mask(uint8_t num_bits, uint8_t bit_field_offset) {
  return ((1UL << num_bits) - 1UL) << bit_field_offset;
}

template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET>
inline constexpr uint32_t get_bit_field_value(const uint32_t& value) {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= 32,
                "Invalid bit field parameters. The total number of bits (offset and the number "
                "of bits in the field) must be less than or equal to the size of the register "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  constexpr uint32_t MASK{compute_bit_mask<NUM_BITS, BIT_FIELD_OFFSET>()};
  return (value & MASK) >> BIT_FIELD_OFFSET;
}

template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET>
inline uint32_t get_bit_field_value(const volatile uint32_t& value) {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= 32,
                "Invalid bit field parameters. The total number of bits (offset and the number "
                "of bits in the field) must be less than or equal to the size of the register "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  constexpr uint32_t MASK{compute_bit_mask<NUM_BITS, BIT_FIELD_OFFSET>()};
  return (value & MASK) >> BIT_FIELD_OFFSET;
}

template <uint8_t NUM_BITS>
inline constexpr uint32_t get_bit_field_value(const uint32_t& value, uint8_t bit_field_offset) {
  const uint32_t MASK{compute_bit_mask<NUM_BITS>(bit_field_offset)};
  return (value & MASK) >> bit_field_offset;
}

template <uint8_t NUM_BITS>
inline uint32_t get_bit_field_value(const volatile uint32_t& value, uint8_t bit_field_offset) {
  const uint32_t MASK{compute_bit_mask<NUM_BITS>(bit_field_offset)};
  return (value & MASK) >> bit_field_offset;
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET>
inline constexpr void modify_bit_field(uint32_t& value, uint32_t bit_field_value) {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= 32,
                "Invalid bit field parameters. The total number of bits (offset and the number "
                "of bits in the field) must be less than or equal to the size of the register "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  constexpr uint32_t MASK{compute_bit_mask<NUM_BITS, BIT_FIELD_OFFSET>()};
  value = ((bit_field_value << BIT_FIELD_OFFSET) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET>
inline void modify_bit_field(volatile uint32_t& value, uint32_t bit_field_value) {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= 32,
                "Invalid bit field parameters. The total number of bits (offset and the number "
                "of bits in the field) must be less than or equal to the size of the register "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  constexpr uint32_t MASK{compute_bit_mask<NUM_BITS, BIT_FIELD_OFFSET>()};
  value = ((bit_field_value << BIT_FIELD_OFFSET) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <uint8_t NUM_BITS>
inline constexpr void modify_bit_field(uint32_t& value, uint32_t bit_field_value,
                                       uint8_t bit_field_offset) {
  const uint32_t MASK{compute_bit_mask<NUM_BITS>(bit_field_offset)};
  value = ((bit_field_value << bit_field_offset) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <uint8_t NUM_BITS>
inline void modify_bit_field(volatile uint32_t& value, uint32_t bit_field_value,
                             uint8_t bit_field_offset) {
  const uint32_t MASK{compute_bit_mask<NUM_BITS>(bit_field_offset)};
  value = ((bit_field_value << bit_field_offset) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
inline constexpr void modify_bit_field(uint32_t& value, uint32_t bit_field_value, uint8_t num_bits,
                                       uint8_t bit_field_offset) {
  const uint32_t MASK{compute_bit_mask(num_bits, bit_field_offset)};
  value = ((bit_field_value << bit_field_offset) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
inline void modify_bit_field(volatile uint32_t& value, uint32_t bit_field_value, uint8_t num_bits,
                             uint8_t bit_field_offset) {
  const uint32_t MASK{compute_bit_mask(num_bits, bit_field_offset)};
  uint32_t result = value;
  value = ((bit_field_value << bit_field_offset) & MASK) | (result & ~MASK);
}

}  // namespace tvsc
