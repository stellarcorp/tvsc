#include "configuration/serialization.h"

#include <cstdint>
#include <string>

#include "configuration/fake_systems.h"
#include "configuration/system_definition.h"
#include "configuration/types.h"
#include "gtest/gtest.h"

namespace tvsc::configuration {

namespace internal {

TEST(InternalSerializationTest, CanRoundTripValueRangeInt32) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const ValueRange<int32_t> in{42, 52};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  ValueRange<int32_t> out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripValueRangeInt64) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const ValueRange<int64_t> in{42L, 52L};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  ValueRange<int64_t> out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripValueRangeFloat) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const ValueRange<float> in{42.f, 52.f};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  ValueRange<float> out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripValueRangeDouble) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const ValueRange<double> in{42., 52.};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  ValueRange<double> out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripInt32DiscreteValue) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const DiscreteValue in{42};
  ASSERT_TRUE(std::holds_alternative<int32_t>(in));

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  DiscreteValue out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripInt64DiscreteValue) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const DiscreteValue in{42L};
  ASSERT_TRUE(std::holds_alternative<int64_t>(in));

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  DiscreteValue out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripFloatDiscreteValue) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const DiscreteValue in{42.f};
  ASSERT_TRUE(std::holds_alternative<float>(in));

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  DiscreteValue out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripDoubleDiscreteValue) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const DiscreteValue in{42.};
  ASSERT_TRUE(std::holds_alternative<double>(in));

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  DiscreteValue out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripStringDiscreteValue) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const DiscreteValue in{"Hello, world!"};
  ASSERT_TRUE(std::holds_alternative<std::string>(in));

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  DiscreteValue out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripRangedValueInt32) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const RangedValue in{std::pair<int32_t, int32_t>{42, 52}};
  ASSERT_TRUE(holds_alternative<int32_t>(in));

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  RangedValue out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripRangedValueInt64) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const RangedValue in{std::pair<int64_t, int64_t>{42L, 52L}};
  ASSERT_TRUE(holds_alternative<int64_t>(in));

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  RangedValue out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripRangedValueFloat) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const RangedValue in{std::pair<float, float>{42.f, 52.f}};
  ASSERT_TRUE(holds_alternative<float>(in));

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  RangedValue out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripRangedValueDouble) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const RangedValue in{std::pair<double, double>{42., 52.}};
  ASSERT_TRUE(holds_alternative<double>(in));

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  RangedValue out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesSingleEnumeratedInt32) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const AllowedValues in{42};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesMultipleEnumeratedInt32) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const AllowedValues in{42, 52, 62};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);

  // Double-check expected behavior.
  EXPECT_TRUE(out.is_allowed(52));
  EXPECT_FALSE(out.is_allowed(43));
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesSingleRangedInt32) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const AllowedValues in{ValueRange<int32_t>{42, 52}};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesMultipleRangedInt32) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const AllowedValues in{ValueRange<int32_t>{42, 52}, ValueRange<int32_t>{55, 100}};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesInt32) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  AllowedValues in{ValueRange<int32_t>{42, 52}, ValueRange<int32_t>{55, 100}};
  in.add_enumerated_value(DiscreteValue{1});
  in.add_enumerated_value(DiscreteValue{3});
  in.add_enumerated_value(DiscreteValue{5});

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);

  // Double-check expected behavior.
  EXPECT_TRUE(out.is_allowed(52));
  EXPECT_TRUE(out.is_allowed(56));
  EXPECT_TRUE(out.is_allowed(3));
  EXPECT_FALSE(out.is_allowed(4));
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesSingleEnumeratedInt64) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const AllowedValues in{42L};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesMultipleEnumeratedInt64) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const AllowedValues in{42L, 52L, 62L};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);

  // Double-check expected behavior.
  EXPECT_TRUE(out.is_allowed(52L));
  EXPECT_FALSE(out.is_allowed(43L));
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesSingleRangedInt64) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const AllowedValues in{ValueRange<int64_t>{42L, 52L}};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesMultipleRangedInt64) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const AllowedValues in{ValueRange<int64_t>{42L, 52L}, ValueRange<int64_t>{55L, 100L}};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(InternalSerializationTest, CanRoundTripAllowedValuesInt64) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  AllowedValues in{ValueRange<int64_t>{42L, 52L}, ValueRange<int64_t>{55L, 100L}};
  in.add_enumerated_value(DiscreteValue{1L});
  in.add_enumerated_value(DiscreteValue{3L});
  in.add_enumerated_value(DiscreteValue{5L});

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  AllowedValues out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);

  // Double-check expected behavior.
  EXPECT_TRUE(out.is_allowed(52L));
  EXPECT_TRUE(out.is_allowed(56L));
  EXPECT_TRUE(out.is_allowed(3L));
  EXPECT_FALSE(out.is_allowed(2L));
  EXPECT_FALSE(out.is_allowed(4L));
}

TEST(InternalSerializationTest, CanRoundTripFunction) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  AllowedValues values{ValueRange<int64_t>{42L, 52L}, ValueRange<int64_t>{55L, 100L}};
  values.add_enumerated_value(DiscreteValue{1L});
  values.add_enumerated_value(DiscreteValue{3L});
  values.add_enumerated_value(DiscreteValue{5L});
  Function in{1, "some_function", values};

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Function out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);

  // Double-check expected behavior.
  EXPECT_TRUE(out.is_allowed(52L));
  EXPECT_TRUE(out.is_allowed(56L));
  EXPECT_TRUE(out.is_allowed(3L));
  EXPECT_FALSE(out.is_allowed(2L));
  EXPECT_FALSE(out.is_allowed(4L));
}

TEST(InternalSerializationTest, CanRoundTripProperty) {
  static constexpr float VALUE{256.f};
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  Property in{42, "some_property", VALUE};

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Property out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);

  // Double-check expected behavior.
  EXPECT_EQ(DiscreteValue{VALUE}, out.value());
}

TEST(InternalSerializationTest, CanRoundTripSetting) {
  static constexpr float VALUE{256.f};
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  Setting in{42, VALUE};

  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Setting out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);

  // Double-check expected behavior.
  EXPECT_EQ(DiscreteValue{VALUE}, out.value());
}

}  // namespace internal

TEST(SerializationTest, CanRoundTripSystemDefinition) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const SystemDefinition& in{satellite_42};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  SystemDefinition out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out);
}

TEST(SerializationTest, CanRoundTripSystemConfiguration) {
  static constexpr size_t BUFFER_SIZE{1024};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const SystemConfiguration& in{satellite_42};
  ASSERT_TRUE(encode(in, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  SystemConfiguration out{};

  ASSERT_TRUE(decode(out, buffer, encoded_size));

  EXPECT_EQ(in, out) << "in: " << to_string(in) << ", out: " << to_string(out);
}

}  // namespace tvsc::configuration
