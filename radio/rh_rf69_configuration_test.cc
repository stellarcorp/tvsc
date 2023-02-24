#include "radio/rh_rf69_configuration.h"

#include "RadioHead/RH_RF69.h"
#include "gmock/gmock.h"
#include "radio/radio_configuration.h"

namespace tvsc::radio {

using ::testing::Contains;
using ::testing::IsSupersetOf;

TEST(RadioConfigurationTest, CanCompile) {
  RH_RF69 driver{};
  RadioConfiguration<RH_RF69> radio{driver};
  EXPECT_TRUE(true);
}

TEST(RadioConfigurationTest, HasId) {
  RH_RF69 driver1{};
  RadioConfiguration<RH_RF69> radio1{driver1};
  RH_RF69 driver2{};
  RadioConfiguration<RH_RF69> radio2{driver2};

  EXPECT_NE(radio1.expanded_id(), radio2.expanded_id());
  EXPECT_NE(radio1.id(), radio2.id());
}

TEST(RadioConfigurationTest, HasDefaultName) {
  RH_RF69 driver{};
  RadioConfiguration<RH_RF69> radio{driver};

  EXPECT_FALSE(radio.name().empty());
}

TEST(RadioConfigurationTest, HasMinimumCapabilityFunctions) {
  RH_RF69 driver{};
  RadioConfiguration<RH_RF69> radio{driver};

  const auto& functions{radio.get_configurable_functions()};
  EXPECT_THAT(functions,
              IsSupersetOf({Function::CARRIER_FREQUENCY_HZ, Function::MODULATION_SCHEME}));
}

TEST(RadioConfigurationTest, CanSetFrequency) {
  constexpr float FREQUENCY{430.f * 1'000'000};
  RH_RF69 driver{};
  RadioConfiguration<RH_RF69> radio{driver};

  radio.set_value(Function::CARRIER_FREQUENCY_HZ, as_discrete_value<float>(FREQUENCY));
  radio.commit_changes();

  EXPECT_EQ(FREQUENCY, as<float>(radio.get_value(Function::CARRIER_FREQUENCY_HZ)));
}

}  // namespace tvsc::radio
