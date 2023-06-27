#include "base/bits.h"

#include "gtest/gtest.h"

namespace tvsc {

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

}  // namespace tvsc
