#include "radio/radio_configuration.h"

#include "RadioHead/RH_RF69.h"
#include "gmock/gmock.h"
#include "radio/radio_head_rf69_configuration.h"

namespace tvsc::radio {

TEST(RadioConfigurationTest, CanCompile) {
  RH_RF69 driver{};
  RadioConfiguration<RH_RF69> radio{driver};
  EXPECT_TRUE(true);
}

}  // namespace tvsc::radio
