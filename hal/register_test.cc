#include "hal/register.h"

#include "bits/bits.h"
#include "gtest/gtest.h"

namespace tvsc::hal {

TEST(ComputeMaskTest, GeneratesDesiredMask8bit) {
  // All of the masks here fit in the least significant 8 bits of the word. This test may help
  // isolate fundamental bugs in the mask computation, but it does not test the full functionality.
  uint32_t mask{};
  mask = bits::compute_bit_mask<8, 0, uint32_t>();
  EXPECT_EQ(0b11111111, mask);
  mask = bits::compute_bit_mask<6, 2, uint32_t>();
  EXPECT_EQ(0b11111100, mask);
  mask = bits::compute_bit_mask<1, 1, uint32_t>();
  EXPECT_EQ(0b00000010, mask);
  mask = bits::compute_bit_mask<1, 0, uint32_t>();
  EXPECT_EQ(0b00000001, mask);
  mask = bits::compute_bit_mask<1, 6, uint32_t>();
  EXPECT_EQ(0b01000000, mask);
  mask = bits::compute_bit_mask<3, 4, uint32_t>();
  EXPECT_EQ(0b01110000, mask);
}

TEST(ComputeMaskTest, GeneratesDesiredMask) {
  uint32_t mask{};
  mask = bits::compute_bit_mask<32, 0, uint32_t>();
  EXPECT_EQ(static_cast<uint32_t>(-1), mask);
  mask = bits::compute_bit_mask<31, 0, uint32_t>();
  EXPECT_EQ(static_cast<uint32_t>(-1) >> 1, mask);
  mask = bits::compute_bit_mask<31, 1, uint32_t>();
  EXPECT_EQ(static_cast<uint32_t>(-1) << 1, mask);
  mask = bits::compute_bit_mask<3, 11, uint32_t>();
  EXPECT_EQ(0b00000000000000000011100000000000, mask);
  mask = bits::compute_bit_mask<8, 16, uint32_t>();
  EXPECT_EQ(0b00000000111111110000000000000000, mask);
}

TEST(ComputeMaskTest, CanComputeMaskAtCompile) {
  static constexpr uint32_t EXPECTED_MASK{0b01110000};
  static constexpr uint32_t mask{bits::compute_bit_mask<3, 4, uint32_t>()};
  static_assert(mask == EXPECTED_MASK, "Invalid mask");
  EXPECT_EQ(EXPECTED_MASK, mask);
}

TEST(RegisterTest, CanSetValue) {
  static constexpr uint32_t EXPECTED_VALUE{0xab};
  volatile Register r{};
  r.set_value(EXPECTED_VALUE);
  EXPECT_EQ(EXPECTED_VALUE, r.value());
}

TEST(RegisterTest, CanSetBitFieldValue) {
  static constexpr uint8_t NUM_FIELD_BITS{3};
  static constexpr uint8_t FIELD_BIT_OFFSET{2};

  static constexpr uint32_t FIELD_VALUE{0x03};
  static constexpr uint32_t INITIAL_REGISTER_VALUE{0b10101010};
  volatile Register r{};
  r.set_value(INITIAL_REGISTER_VALUE);

  uint32_t field_value;

  field_value = r.bit_field_value<NUM_FIELD_BITS, FIELD_BIT_OFFSET>();
  EXPECT_EQ(2, field_value);

  r.set_bit_field_value<NUM_FIELD_BITS, FIELD_BIT_OFFSET>(0);
  field_value = r.bit_field_value<NUM_FIELD_BITS, FIELD_BIT_OFFSET>();
  EXPECT_EQ(0, field_value);
  EXPECT_EQ(0xa2, r.value());

  r.set_bit_field_value<NUM_FIELD_BITS, FIELD_BIT_OFFSET>(FIELD_VALUE);
  field_value = r.bit_field_value<NUM_FIELD_BITS, FIELD_BIT_OFFSET>();
  EXPECT_EQ(FIELD_VALUE, field_value);
  EXPECT_EQ(0xae, r.value());
}

}  // namespace tvsc::hal
