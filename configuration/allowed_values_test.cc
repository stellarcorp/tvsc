#include "configuration/allowed_values.h"

#include "configuration/fake_systems.h"
#include "configuration/types.h"
#include "gtest/gtest.h"

namespace tvsc::configuration {

TEST(SettingsUsabilityTest, CanTestAllowedDiscreteValue) {
  static AllowedValues modulation_scheme_values{ModulationScheme::FSK, ModulationScheme::OOK};

  EXPECT_TRUE(modulation_scheme_values.is_allowed(ModulationScheme::OOK));
  EXPECT_TRUE(modulation_scheme_values.is_allowed(ModulationScheme::FSK));
  EXPECT_FALSE(modulation_scheme_values.is_allowed(ModulationScheme::LORA));
}

TEST(SettingsUsabilityTest, CanTestAllowedRangedValue) {
  static AllowedValues rx_sensitivity_values{ValueRange<float>{-127.f, -0.f}};

  EXPECT_TRUE(rx_sensitivity_values.is_allowed(-100.f));
  EXPECT_FALSE(rx_sensitivity_values.is_allowed(-150.f));
}

TEST(SettingsUsabilityTest, Int32RangeIncludesMaxOfRange) {
  static AllowedValues allowed_values{ValueRange<int32_t>{-127, -0}};

  EXPECT_TRUE(allowed_values.is_allowed(-0));
}

TEST(SettingsUsabilityTest, Int64RangeIncludesMaxOfRange) {
  static AllowedValues allowed_values{ValueRange<int64_t>{-127L, -0L}};

  EXPECT_TRUE(allowed_values.is_allowed(-0L));
}

TEST(SettingsUsabilityTest, FloatRangeExcludesMaxOfRange) {
  static AllowedValues allowed_values{ValueRange<float>{-127.f, -0.f}};

  EXPECT_FALSE(allowed_values.is_allowed(-0.f));
}

TEST(SettingsUsabilityTest, DoubleRangeExcludesMaxOfRange) {
  static AllowedValues allowed_values{ValueRange<double>{-127., -0.}};

  EXPECT_FALSE(allowed_values.is_allowed(-0.));
}

TEST(SettingsUsabilityTest, Int32ValueTestedAgainstInt64Range) {
  static AllowedValues allowed_values{ValueRange<int64_t>{-127L, -0L}};

  EXPECT_FALSE(allowed_values.is_allowed(-150));
  EXPECT_TRUE(allowed_values.is_allowed(-100));
  EXPECT_TRUE(allowed_values.is_allowed(-0));
}

TEST(SettingsUsabilityTest, FloatValueTestedAgainstDoubleRange) {
  static AllowedValues allowed_values{ValueRange<double>{-127., -0.}};

  EXPECT_FALSE(allowed_values.is_allowed(-150.f));
  EXPECT_TRUE(allowed_values.is_allowed(-100.f));
  EXPECT_FALSE(allowed_values.is_allowed(-0.f));
}

TEST(SettingsUsabilityTest, AllowMultipleRanges) {
  static AllowedValues allowed_values{ValueRange<float>{-127.f, -0.f},
                                      ValueRange<float>{128.f, 255.f}};

  EXPECT_TRUE(allowed_values.is_allowed(-100.f));
  EXPECT_FALSE(allowed_values.is_allowed(-0.f));
  EXPECT_FALSE(allowed_values.is_allowed(1.f));
  EXPECT_TRUE(allowed_values.is_allowed(200.f));
}

}  // namespace tvsc::configuration
