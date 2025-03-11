#include "string/base64.h"

#include <limits>

#include "gtest/gtest.h"

namespace tvsc::string {

void test_round_trip(uint64_t value) {
  std::string encoded = Base64::encode(value);
  uint64_t decoded = Base64::decode(encoded);
  EXPECT_EQ(value, decoded) << "Failed for value: " << value;
}

void test_round_trip(const std::string& value) {
  uint64_t decoded = Base64::decode(value);
  std::string encoded = Base64::encode(decoded);
  EXPECT_EQ(value, encoded) << "Failed for value: " << value;
}

TEST(Base64Test, CanRoundtripSelectValues) {
  test_round_trip(123456789);
  test_round_trip(987654321);
  test_round_trip(0);
  test_round_trip(std::numeric_limits<uint64_t>::max());
  test_round_trip("A");
  test_round_trip("Z");
  test_round_trip("a");
  test_round_trip("z");
  test_round_trip("0");
  test_round_trip("9");
  test_round_trip("-");
  test_round_trip("_");
}

TEST(Base64Test, CanRoundtripSmallValues) {
  static constexpr uint64_t MAX_TEST_VALUE{256};
  for (uint64_t i = 0; i < MAX_TEST_VALUE; ++i) {
    test_round_trip(i);
  }
}

TEST(Base64Test, ThrowsOnInvalidValues) {
  EXPECT_THROW(Base64::decode("Invalid@String"), std::invalid_argument);
  EXPECT_THROW(Base64::decode("123#"), std::invalid_argument);
}

TEST(Base64Test, HandlesSelectEdgeCases) {
  EXPECT_EQ("B", Base64::encode(1));
  EXPECT_EQ(1, Base64::decode("B"));

  EXPECT_EQ("BA", Base64::encode(64));
  EXPECT_EQ(64, Base64::decode("BA"));
}

TEST(Base64Test, CanEncodeZero) {
  uint64_t zero{0};
  const std::string encoded{Base64::encode(zero)};
  EXPECT_EQ("A", encoded);
}

TEST(Base64Test, CanDecodeWithPrependedZero) {
  const std::string base64_zero{"A"};
  const std::string values[] = {"A", "B", "BAA", "42"};
  for (const auto& value : values) {
    std::string with_prepended_zeros{base64_zero + value};
    while (with_prepended_zeros.size() <= Base64::MAX_ENCODED_STRING_SIZE) {
      EXPECT_EQ(value, Base64::encode(Base64::decode(with_prepended_zeros)));
      with_prepended_zeros = base64_zero + with_prepended_zeros;
    }
  }
}

}  // namespace tvsc::string
