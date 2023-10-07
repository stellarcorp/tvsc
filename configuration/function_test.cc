#include "configuration/system_definition.h"
#include "configuration/fake_systems.h"
#include "gtest/gtest.h"

namespace tvsc::configuration {

TEST(FunctionTest, CanTestAllowedValue) {
  EXPECT_TRUE(modulation_scheme_values.is_allowed(ModulationScheme::OOK));
  EXPECT_TRUE(modulation_scheme_values.is_allowed(ModulationScheme::FSK));
  EXPECT_FALSE(modulation_scheme_values.is_allowed(ModulationScheme::LORA));
}

TEST(FunctionTest, CanTestAllowedValueForFunction) {
  EXPECT_TRUE(modulation_scheme.is_allowed(ModulationScheme::OOK));
  EXPECT_TRUE(modulation_scheme.is_allowed(ModulationScheme::FSK));
  EXPECT_FALSE(modulation_scheme.is_allowed(ModulationScheme::LORA));
}

}  // namespace tvsc::configuration
