#pragma once

#include <cstdint>

#ifdef __has_include
#if __has_include(<bit>)
#include <bit>
#endif
#endif

namespace tvsc::bits {

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

// This method is effectively a typed form of the literal 1. Using this method avoids a number of
// compiler warnings and allows the compiler to generate code appropriate for the result type
// without repeating the idiom in several methods below.
template <typename ResultType>
inline constexpr ResultType one() {
  return static_cast<ResultType>(1);
}

template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET, typename ResultType>
inline constexpr ResultType compute_bit_mask() {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= sizeof(ResultType) * 8,
                "Invalid mask parameters. The total number of bits (offset and the number "
                "of bits in the mask) must be less than or equal to the size of the ResultType. "
                "Likely, this is a typo or other scrivener's error in the template parameters to "
                "call this method.");
  return ((one<ResultType>() << NUM_BITS) - one<ResultType>()) << BIT_FIELD_OFFSET;
}

template <>
inline constexpr uint32_t compute_bit_mask<32, 0, uint32_t>() {
  return static_cast<uint32_t>(-1);
}

template <>
inline constexpr uint16_t compute_bit_mask<16, 0, uint16_t>() {
  return static_cast<uint16_t>(-1);
}

template <>
inline constexpr uint8_t compute_bit_mask<8, 0, uint8_t>() {
  return static_cast<uint8_t>(-1);
}

template <uint8_t NUM_BITS, typename ResultType = uint32_t>
inline constexpr ResultType compute_bit_mask(uint8_t bit_field_offset) {
  return ((one<ResultType>() << NUM_BITS) - one<ResultType>()) << bit_field_offset;
}

template <typename ResultType = uint32_t>
inline constexpr ResultType compute_bit_mask(uint8_t num_bits, uint8_t bit_field_offset) {
  return ((one<ResultType>() << num_bits) - one<ResultType>()) << bit_field_offset;
}

template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET, typename ResultType = uint32_t>
inline constexpr ResultType get_bit_field_value(const ResultType& value) {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= sizeof(ResultType) * 8,
                "Invalid bit field parameters. The total number of bits (offset and the number "
                "of bits in the field) must be less than or equal to the size of the register "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  constexpr ResultType MASK{compute_bit_mask<NUM_BITS, BIT_FIELD_OFFSET, ResultType>()};
  return (value & MASK) >> BIT_FIELD_OFFSET;
}

template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET, typename ResultType = uint32_t>
inline ResultType get_bit_field_value(const volatile ResultType& value) {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= sizeof(ResultType) * 8,
                "Invalid bit field parameters. The total number of bits (offset and the number "
                "of bits in the field) must be less than or equal to the size of the register "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  constexpr ResultType MASK{compute_bit_mask<NUM_BITS, BIT_FIELD_OFFSET, ResultType>()};
  return (value & MASK) >> BIT_FIELD_OFFSET;
}

template <uint8_t NUM_BITS, typename ResultType = uint32_t>
inline constexpr ResultType get_bit_field_value(const ResultType& value, uint8_t bit_field_offset) {
  const ResultType MASK{compute_bit_mask<NUM_BITS, ResultType>(bit_field_offset)};
  return (value & MASK) >> bit_field_offset;
}

template <uint8_t NUM_BITS, typename ResultType = uint32_t>
inline ResultType get_bit_field_value(const volatile ResultType& value, uint8_t bit_field_offset) {
  const ResultType MASK{compute_bit_mask<NUM_BITS, ResultType>(bit_field_offset)};
  return (value & MASK) >> bit_field_offset;
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET, typename ResultType = uint32_t>
inline constexpr void modify_bit_field(ResultType& value, ResultType bit_field_value) {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= sizeof(ResultType) * 8,
                "Invalid bit field parameters. The total number of bits (offset and the number "
                "of bits in the field) must be less than or equal to the size of the register "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  constexpr ResultType MASK{compute_bit_mask<NUM_BITS, BIT_FIELD_OFFSET, ResultType>()};
  value = ((bit_field_value << BIT_FIELD_OFFSET) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET, typename ResultType = uint32_t>
inline void modify_bit_field(volatile ResultType& value, ResultType bit_field_value) {
  static_assert(NUM_BITS + BIT_FIELD_OFFSET <= sizeof(ResultType) * 8,
                "Invalid bit field parameters. The total number of bits (offset and the number "
                "of bits in the field) must be less than or equal to the size of the register "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  constexpr ResultType MASK{compute_bit_mask<NUM_BITS, BIT_FIELD_OFFSET, ResultType>()};
  value = ((bit_field_value << BIT_FIELD_OFFSET) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <uint8_t NUM_BITS, typename ResultType = uint32_t>
inline constexpr void modify_bit_field(ResultType& value, ResultType bit_field_value,
                                       uint8_t bit_field_offset) {
  const ResultType MASK{compute_bit_mask<NUM_BITS, ResultType>(bit_field_offset)};
  value = ((bit_field_value << bit_field_offset) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <uint8_t NUM_BITS, typename ResultType = uint32_t>
inline void modify_bit_field(volatile ResultType& value, ResultType bit_field_value,
                             uint8_t bit_field_offset) {
  const ResultType MASK{compute_bit_mask<NUM_BITS, ResultType>(bit_field_offset)};
  value = ((bit_field_value << bit_field_offset) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <typename ResultType = uint32_t>
inline constexpr void modify_bit_field(ResultType& value, ResultType bit_field_value,
                                       uint8_t num_bits, uint8_t bit_field_offset) {
  const ResultType MASK{compute_bit_mask<ResultType>(num_bits, bit_field_offset)};
  value = ((bit_field_value << bit_field_offset) & MASK) | (value & ~MASK);
}

/**
 * Set a particular set of bits in a value, leaving the others unchanged.
 */
template <typename ResultType = uint32_t>
inline void modify_bit_field(volatile ResultType& value, ResultType bit_field_value,
                             uint8_t num_bits, uint8_t bit_field_offset) {
  const ResultType MASK{compute_bit_mask<ResultType>(num_bits, bit_field_offset)};
  ResultType result = value;
  value = ((bit_field_value << bit_field_offset) & MASK) | (result & ~MASK);
}

}  // namespace tvsc::bits
