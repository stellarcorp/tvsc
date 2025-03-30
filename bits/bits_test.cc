#include "bits/bits.h"

#include "gtest/gtest.h"

namespace tvsc::bits {

TEST(BitWidthTest, ZeroRequiresZeroBits) { EXPECT_EQ(0, bit_width(0)); }

TEST(BitWidthTest, OneRequiresOneBit) { EXPECT_EQ(1, bit_width(1)); }

TEST(BitWidthTest, TwoAndThreeRequireTwoBits) {
  EXPECT_EQ(2, bit_width(2));
  EXPECT_EQ(2, bit_width(3));
}

TEST(BitWidthTest, VerifyAssortedValues) {
  EXPECT_EQ(7, bit_width(64));
  EXPECT_EQ(7, bit_width(65));
  EXPECT_EQ(7, bit_width(127));
  EXPECT_EQ(8, bit_width(128));
  EXPECT_EQ(8, bit_width(255));
  EXPECT_EQ(10, bit_width(1023));
  EXPECT_EQ(11, bit_width(1024));
}

template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET, typename ValueType>
::testing::AssertionResult CanComputeBitMask(ValueType expected_value) {
  static constexpr uint32_t compile_time_value{
      compute_bit_mask<NUM_BITS, BIT_FIELD_OFFSET, ValueType>()};
  const uint32_t partial_compile_time_value{
      compute_bit_mask<NUM_BITS, ValueType>(BIT_FIELD_OFFSET)};
  const uint32_t run_time_value{compute_bit_mask<ValueType>(NUM_BITS, BIT_FIELD_OFFSET)};

  if (expected_value != compile_time_value) {
    return ::testing::AssertionFailure();
  } else if (expected_value != partial_compile_time_value) {
    return ::testing::AssertionFailure();
  } else if (expected_value != run_time_value) {
    return ::testing::AssertionFailure();
  } else {
    return ::testing::AssertionSuccess();
  }
}

TEST(BitMask32Test, CanComputeBitMasks) {
  ::testing::AssertionResult result{::testing::AssertionSuccess()};
  result = CanComputeBitMask<2, 4>(0b00110000);
  EXPECT_TRUE(result);
  result = CanComputeBitMask<1, 4>(0b00010000);
  EXPECT_TRUE(result);
  result = CanComputeBitMask<17, 4>(0b00000000000111111111111111110000);
  EXPECT_TRUE(result);
  result = CanComputeBitMask<4, 17>(0b00000000000111100000000000000000);
  EXPECT_TRUE(result);
  result = CanComputeBitMask<5, 17>(0b00000000001111100000000000000000);
  EXPECT_TRUE(result);
}

template <uint8_t NUM_BITS, uint8_t BIT_FIELD_OFFSET, typename ValueType = uint32_t>
::testing::AssertionResult CanModifyBitField() {
  static constexpr ValueType ZEROS{static_cast<ValueType>(0x00000000)};
  static constexpr ValueType ONES{static_cast<ValueType>(0xffffffff)};
  static constexpr ValueType EVENS{static_cast<ValueType>(0xaaaaaaaa)};
  static constexpr ValueType ODDS{static_cast<ValueType>(0x55555555)};
  static constexpr ValueType RANDOM{
      static_cast<ValueType>(0b1010'0011'1101'1001'1101'1010'1011'0011)};

  static_assert(ONES == EVENS + ODDS,
                "Bug in either EVENS or ODDS. They aren't bit patterns we expect.");

  ValueType zeros_modified{ZEROS};
  ValueType ones_modified{ONES};
  ValueType evens_modified{EVENS};
  ValueType odds_modified{ODDS};
  ValueType random_modified{RANDOM};

  for (int i = 0; i < 5; ++i) {
    ValueType temp{zeros_modified};
    modify_bit_field<NUM_BITS, BIT_FIELD_OFFSET, ValueType>(
        zeros_modified, get_bit_field_value<NUM_BITS, BIT_FIELD_OFFSET>(random_modified));
    modify_bit_field<NUM_BITS, BIT_FIELD_OFFSET, ValueType>(
        random_modified, get_bit_field_value<NUM_BITS, BIT_FIELD_OFFSET>(odds_modified));
    modify_bit_field<NUM_BITS, BIT_FIELD_OFFSET, ValueType>(
        odds_modified, get_bit_field_value<NUM_BITS, BIT_FIELD_OFFSET>(evens_modified));
    modify_bit_field<NUM_BITS, BIT_FIELD_OFFSET, ValueType>(
        evens_modified, get_bit_field_value<NUM_BITS, BIT_FIELD_OFFSET>(ones_modified));
    modify_bit_field<NUM_BITS, BIT_FIELD_OFFSET, ValueType>(
        ones_modified, get_bit_field_value<NUM_BITS, BIT_FIELD_OFFSET>(temp));
  }

  if (zeros_modified != ZEROS) {
    return ::testing::AssertionFailure();
  } else if (ones_modified != ONES) {
    return ::testing::AssertionFailure();
  } else if (evens_modified != EVENS) {
    return ::testing::AssertionFailure();
  } else if (odds_modified != ODDS) {
    return ::testing::AssertionFailure();
  } else if (random_modified != RANDOM) {
    return ::testing::AssertionFailure();
  } else {
    return ::testing::AssertionSuccess();
  }
}

TEST(BitField32Test, CanModifyBitFields) {
  ::testing::AssertionResult result{::testing::AssertionSuccess()};
  result = CanModifyBitField<1, 0>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<2, 0>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<1, 2>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<1, 3>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<6, 12>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<11, 13>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<13, 17>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<13, 19>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<3, 27>();
  EXPECT_TRUE(result);
}

TEST(BitMask16Test, CanComputeBitMasks) {
  ::testing::AssertionResult result{::testing::AssertionSuccess()};
  result = CanComputeBitMask<2, 4, uint16_t>(0b00110000);
  EXPECT_TRUE(result);
  result = CanComputeBitMask<1, 4, uint16_t>(0b00010000);
  EXPECT_TRUE(result);
  result = CanComputeBitMask<11, 4, uint16_t>(0b111111111110000);
  EXPECT_TRUE(result);
}

TEST(BitField16Test, CanModifyBitFields) {
  ::testing::AssertionResult result{::testing::AssertionSuccess()};
  result = CanModifyBitField<1, 0, uint16_t>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<2, 0, uint16_t>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<1, 2, uint16_t>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<1, 3, uint16_t>();
  EXPECT_TRUE(result);
  result = CanModifyBitField<3, 11, uint16_t>();
  EXPECT_TRUE(result);
}

}  // namespace tvsc::bits
