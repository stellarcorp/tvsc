#include <limits>

#include "base/units.h"
#include "gmock/gmock.h"
#include "radio/radio_configuration.h"
#include "radio/settings.h"
#include "radio/radio.pb.h"

namespace tvsc::radio {

TEST(RadioSettingsTest, CanIdentifyValidInt32ValuesInEnumeratedCapability) {
  const auto& allowed_values{enumerated({0, 1, 2, 3, 4})};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(1)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(2)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(3)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(4)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(5)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt32ValuesInRangedCapability) {
  const auto& allowed_values{int32_range(0, 65535)};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(65536)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt32ValuesInRangedCapabilityInclusiveInclusive) {
  const auto& allowed_values{int32_range(0, 65535, RangeInclusivity::INCLUSIVE_INCLUSIVE)};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(65536)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt32ValuesInRangedCapabilityInclusiveExclusive) {
  const auto& allowed_values{int32_range(0, 65535, RangeInclusivity::INCLUSIVE_EXCLUSIVE)};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(65535)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(65536)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt32ValuesInRangedCapabilityExclusiveInclusive) {
  const auto& allowed_values{int32_range(0, 65535, RangeInclusivity::EXCLUSIVE_INCLUSIVE)};
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(0)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(65536)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt32ValuesInRangedCapabilityExclusiveExclusive) {
  const auto& allowed_values{int32_range(0, 65535, RangeInclusivity::EXCLUSIVE_EXCLUSIVE)};
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(0)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(65535)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(65536)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt64ValuesInEnumeratedCapability) {
  const auto& allowed_values{
      enumerated({0L, 1L, 2L, 3L, 4L, std::numeric_limits<long int>::max()})};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(1L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(2L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(3L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(4L)));
  EXPECT_TRUE(
      is_valid_setting(allowed_values, as_discrete_value(std::numeric_limits<long int>::max())));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(5L)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt64ValuesInRangedCapability) {
  const auto& allowed_values{int64_range(0, (1L << 40))};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(1L << 40)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1L << 40) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1L)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt64ValuesInRangedCapabilityInclusiveInclusive) {
  const auto& allowed_values{int64_range(0, (1L << 40), RangeInclusivity::INCLUSIVE_INCLUSIVE)};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(1L << 40)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1L << 40) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1L)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt64ValuesInRangedCapabilityInclusiveExclusive) {
  const auto& allowed_values{int64_range(0, (1L << 40), RangeInclusivity::INCLUSIVE_EXCLUSIVE)};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536L)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(1L << 40)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1L << 40) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1L)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt64ValuesInRangedCapabilityExclusiveInclusive) {
  const auto& allowed_values{int64_range(0, (1L << 40), RangeInclusivity::EXCLUSIVE_INCLUSIVE)};
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(0L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(1L << 40)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1L << 40) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1L)));
}

TEST(RadioSettingsTest, CanIdentifyValidInt64ValuesInRangedCapabilityExclusiveExclusive) {
  const auto& allowed_values{int64_range(0, (1L << 40), RangeInclusivity::EXCLUSIVE_EXCLUSIVE)};
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(0L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535L)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536L)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(1L << 40)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1L << 40) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1L)));
}

TEST(RadioSettingsTest, CanIdentifyValidFloatValuesInRangedCapability) {
  const auto& allowed_values{float_range(0.f, 1e7f)};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(1e7f)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1e7f) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1.f)));
}

TEST(RadioSettingsTest, CanIdentifyValidFloatValuesInRangedCapabilityInclusiveInclusive) {
  const auto& allowed_values{float_range(0.f, 1e7f, RangeInclusivity::INCLUSIVE_INCLUSIVE)};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(1e7f)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1e7f) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1.f)));
}

TEST(RadioSettingsTest, CanIdentifyValidFloatValuesInRangedCapabilityInclusiveExclusive) {
  const auto& allowed_values{float_range(0.f, 1e7f, RangeInclusivity::INCLUSIVE_EXCLUSIVE)};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(0.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536.f)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(1e7f)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1e7f) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1.f)));
}

TEST(RadioSettingsTest, CanIdentifyValidFloatValuesInRangedCapabilityExclusiveInclusive) {
  const auto& allowed_values{float_range(0.f, 1e7f, RangeInclusivity::EXCLUSIVE_INCLUSIVE)};
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(0.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(1e7f)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1e7f) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1.f)));
}

TEST(RadioSettingsTest, CanIdentifyValidFloatValuesInRangedCapabilityExclusiveExclusive) {
  const auto& allowed_values{float_range(0.f, 1e7f, RangeInclusivity::EXCLUSIVE_EXCLUSIVE)};
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(0.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(23.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65535.f)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(65536.f)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(1e7f)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value((1e7f) + 1)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(-1.f)));
}

TEST(RadioSettingsTest, CanIdentifyValidEnumerationValuesInEnumeratedCapability) {
  const auto& allowed_values{enumerated({
      ModulationTechnique::OOK,
      ModulationTechnique::FSK,
      ModulationTechnique::GFSK,
      ModulationTechnique::MSK,
      ModulationTechnique::GMSK,
  })};
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(ModulationTechnique::OOK)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(ModulationTechnique::MSK)));
  EXPECT_TRUE(is_valid_setting(allowed_values, as_discrete_value(ModulationTechnique::GFSK)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(ModulationTechnique::QPSK)));
  EXPECT_FALSE(is_valid_setting(allowed_values, as_discrete_value(ModulationTechnique::DSSS)));
}

}  // namespace tvsc::radio
