#include "configuration/allowed_values.h"

#include "configuration/fake_systems.h"
#include "configuration/types.h"
#include "gtest/gtest.h"

namespace tvsc::configuration {

TEST(AllowedValuesTest, CanTestAllowedDiscreteValue) {
  static AllowedValues modulation_scheme_values{ModulationScheme::FSK, ModulationScheme::OOK};

  EXPECT_TRUE(modulation_scheme_values.is_allowed(ModulationScheme::OOK));
  EXPECT_TRUE(modulation_scheme_values.is_allowed(ModulationScheme::FSK));
  EXPECT_FALSE(modulation_scheme_values.is_allowed(ModulationScheme::LORA));
}

TEST(AllowedValuesTest, CanTestAllowedDiscreteValueInt32s) {
  static AllowedValues modulation_scheme_values{1, 3, 4};

  EXPECT_TRUE(modulation_scheme_values.is_allowed(1));
  EXPECT_FALSE(modulation_scheme_values.is_allowed(2));
  EXPECT_TRUE(modulation_scheme_values.is_allowed(4));
}

TEST(AllowedValuesTest, CanTestAllowedDiscreteValueInt64s) {
  static AllowedValues modulation_scheme_values{1L, 3L, 4L};

  EXPECT_TRUE(modulation_scheme_values.is_allowed(1L));
  EXPECT_FALSE(modulation_scheme_values.is_allowed(2L));
  EXPECT_TRUE(modulation_scheme_values.is_allowed(4L));
}

TEST(AllowedValuesTest, CanTestAllowedDiscreteValueFloats) {
  static AllowedValues modulation_scheme_values{1.f, 3.f, 4.f};

  EXPECT_TRUE(modulation_scheme_values.is_allowed(1.f));
  EXPECT_FALSE(modulation_scheme_values.is_allowed(2.f));
  EXPECT_TRUE(modulation_scheme_values.is_allowed(4.f));
}

TEST(AllowedValuesTest, CanTestAllowedDiscreteValueDoubles) {
  static AllowedValues modulation_scheme_values{1., 3., 4.};

  EXPECT_TRUE(modulation_scheme_values.is_allowed(1.));
  EXPECT_FALSE(modulation_scheme_values.is_allowed(2.));
  EXPECT_TRUE(modulation_scheme_values.is_allowed(4.));
}

TEST(AllowedValuesTest, CanTestAllowedRangedValue) {
  static AllowedValues rx_sensitivity_values{ValueRange<float>{-127.f, -0.f}};

  EXPECT_TRUE(rx_sensitivity_values.is_allowed(-100.f));
  EXPECT_FALSE(rx_sensitivity_values.is_allowed(-150.f));
}

TEST(AllowedValuesTest, Int32RangeIncludesMaxOfRange) {
  static AllowedValues allowed_values{ValueRange<int32_t>{-127, -0}};

  EXPECT_TRUE(allowed_values.is_allowed(-0));
}

TEST(AllowedValuesTest, Int64RangeIncludesMaxOfRange) {
  static AllowedValues allowed_values{ValueRange<int64_t>{-127L, -0L}};

  EXPECT_TRUE(allowed_values.is_allowed(-0L));
}

TEST(AllowedValuesTest, FloatRangeExcludesMaxOfRange) {
  static AllowedValues allowed_values{ValueRange<float>{-127.f, -0.f}};

  EXPECT_FALSE(allowed_values.is_allowed(-0.f));
}

TEST(AllowedValuesTest, DoubleRangeExcludesMaxOfRange) {
  static AllowedValues allowed_values{ValueRange<double>{-127., -0.}};

  EXPECT_FALSE(allowed_values.is_allowed(-0.));
}

TEST(AllowedValuesTest, Int32ValueTestedAgainstInt64Range) {
  static AllowedValues allowed_values{ValueRange<int64_t>{-127L, -0L}};

  EXPECT_FALSE(allowed_values.is_allowed(-150));
  EXPECT_TRUE(allowed_values.is_allowed(-100));
  EXPECT_TRUE(allowed_values.is_allowed(-0));
}

TEST(AllowedValuesTest, FloatValueTestedAgainstDoubleRange) {
  static AllowedValues allowed_values{ValueRange<double>{-127., -0.}};

  EXPECT_FALSE(allowed_values.is_allowed(-150.f));
  EXPECT_TRUE(allowed_values.is_allowed(-100.f));
  EXPECT_FALSE(allowed_values.is_allowed(-0.f));
}

TEST(AllowedValuesTest, AllowMultipleRanges) {
  static AllowedValues allowed_values{ValueRange<float>{-127.f, -0.f},
                                      ValueRange<float>{128.f, 255.f}};

  EXPECT_TRUE(allowed_values.is_allowed(-100.f));
  EXPECT_FALSE(allowed_values.is_allowed(-0.f));
  EXPECT_FALSE(allowed_values.is_allowed(1.f));
  EXPECT_TRUE(allowed_values.is_allowed(200.f));
}

TEST(AllowedValuesTest, HasToStringMethods) {
  static AllowedValues allowed_values_range_float{ValueRange<float>{-127.f, -0.f},
                                                  ValueRange<float>{128.f, 255.f}};
  static AllowedValues allowed_values_range_double{ValueRange<double>{-127., -0.},
                                                   ValueRange<double>{128., 255.}};
  static AllowedValues allowed_values_range_int32{ValueRange<int32_t>{-127, -0},
                                                  ValueRange<int32_t>{128, 255}};
  static AllowedValues allowed_values_range_int64{ValueRange<int64_t>{-127L, -0L},
                                                  ValueRange<int64_t>{128L, 255L}};

  static AllowedValues allowed_values_discrete_float{-127.f, -0.f, 128.f, 255.f};
  static AllowedValues allowed_values_discrete_double{-127., -0., 128., 255.};
  static AllowedValues allowed_values_discrete_int32{-127, -0, 128, 255};
  static AllowedValues allowed_values_discrete_int64{-127L, -0L, 128L, 255L};

  static auto double_range{ValueRange<double>{-127., -0.}};
  static auto float_range{ValueRange<float>{-127.f, -0.f}};
  static auto int32_range{ValueRange<int32_t>{-127, -0}};
  static auto int64_range{ValueRange<int64_t>{-127L, -0L}};

  EXPECT_FALSE(to_string(allowed_values_range_float).empty());
  EXPECT_FALSE(to_string(allowed_values_range_double).empty());
  EXPECT_FALSE(to_string(allowed_values_range_int32).empty());
  EXPECT_FALSE(to_string(allowed_values_range_int64).empty());

  EXPECT_FALSE(to_string(allowed_values_discrete_float).empty());
  EXPECT_FALSE(to_string(allowed_values_discrete_double).empty());
  EXPECT_FALSE(to_string(allowed_values_discrete_int32).empty());
  EXPECT_FALSE(to_string(allowed_values_discrete_int64).empty());

  EXPECT_FALSE(to_string(double_range).empty());
  EXPECT_FALSE(to_string(float_range).empty());
  EXPECT_FALSE(to_string(int32_range).empty());
  EXPECT_FALSE(to_string(int64_range).empty());
}

}  // namespace tvsc::configuration
