#include "radio/rf69hcw_configuration.h"

#include "gmock/gmock.h"
#include "radio/radio_configuration.h"

namespace tvsc::radio {

using ::testing::Contains;
using ::testing::IsSupersetOf;

TEST(RadioConfigurationTest, CanCompile) {
  RF69HCW driver{};
  RadioConfiguration<RF69HCW> radio{driver};
  EXPECT_TRUE(true);
}

TEST(RadioConfigurationTest, HasId) {
  RF69HCW driver1{};
  RadioConfiguration<RF69HCW> radio1{driver1};
  RF69HCW driver2{};
  RadioConfiguration<RF69HCW> radio2{driver2};

  EXPECT_NE(radio1.expanded_id(), radio2.expanded_id());
  EXPECT_NE(radio1.id(), radio2.id());
}

TEST(RadioConfigurationTest, HasDefaultName) {
  RF69HCW driver{};
  RadioConfiguration<RF69HCW> radio{driver};

  EXPECT_FALSE(radio.name().empty());
}

TEST(RadioConfigurationTest, HasMinimumCapabilityFunctions) {
  RF69HCW driver{};
  RadioConfiguration<RF69HCW> radio{driver};

  const auto& functions{radio.get_configurable_functions()};
  EXPECT_THAT(functions,
              IsSupersetOf({Function::CARRIER_FREQUENCY_HZ, Function::MODULATION_SCHEME}));
}

TEST(RadioConfigurationTest, CanSetFrequency) {
  constexpr float FREQUENCY{430.f * 1'000'000};
  RF69HCW driver{};
  RadioConfiguration<RF69HCW> radio{driver};

  radio.set_value(Function::CARRIER_FREQUENCY_HZ, as_discrete_value<float>(FREQUENCY));
  radio.commit_changes();

  EXPECT_EQ(FREQUENCY, as<float>(radio.get_value(Function::CARRIER_FREQUENCY_HZ)));
}

}  // namespace tvsc::radio
