#include "configuration/system_configuration.h"

#include "configuration/allowed_values.h"
#include "configuration/fake_systems.h"
#include "configuration/system_definition.h"
#include "configuration/types.h"
#include "gtest/gtest.h"

namespace tvsc::configuration {

TEST(ConfigurationTest, CanDetectValidConfiguration) {
  EXPECT_TRUE(is_valid_configuration(satellite_42, satellite_42_configuration_1));
  EXPECT_TRUE(is_valid_configuration(satellite_42, satellite_42_configuration_3));
}

TEST(ConfigurationTest, CanDetectInvalidConfiguration) {
  EXPECT_FALSE(is_valid_configuration(satellite_42, satellite_42_configuration_2));
}

TEST(ConfigurationUsabilityTest, CanGenerateString) {
  EXPECT_FALSE(to_string(satellite_42_configuration_1).empty());
}

}  // namespace tvsc::configuration
