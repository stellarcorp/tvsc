#include "configuration/allowed_values.h"
#include "configuration/utility.h"
#include "gtest/gtest.h"

namespace tvsc::configuration {

enum class ModulationScheme {
  CW,
  OOK,
  FSK,
  LORA,
};

TEST(SettingsUsabilityTest, CanTestAllowedValue) {
  static CONSTEXPR_SETTINGS AllowedValues modulation_scheme_values{ModulationScheme::FSK, ModulationScheme::OOK};
#if __cplusplus >= 202000
  // We expect to be able to check if particular values are allowed at compile time in C++20 and
  // later.
  static_assert(modulation_scheme_values.is_allowed(ModulationScheme::OOK));
  static_assert(modulation_scheme_values.is_allowed(ModulationScheme::FSK));
  static_assert(!modulation_scheme_values.is_allowed(ModulationScheme::LORA));
#endif

  EXPECT_TRUE(modulation_scheme_values.is_allowed(ModulationScheme::OOK));
  EXPECT_TRUE(modulation_scheme_values.is_allowed(ModulationScheme::FSK));
  EXPECT_FALSE(modulation_scheme_values.is_allowed(ModulationScheme::LORA));
}

}  // namespace tvsc::configuration
