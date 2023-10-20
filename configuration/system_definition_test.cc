#include "configuration/system_definition.h"

#include "configuration/allowed_values.h"
#include "configuration/fake_systems.h"
#include "configuration/types.h"
#include "gtest/gtest.h"

namespace tvsc::configuration {

TEST(SystemDefinitionUsabilityTest, CanTestAllowedValueForFunction) {
  EXPECT_TRUE(modulation_scheme.is_allowed(ModulationScheme::OOK));
  EXPECT_TRUE(modulation_scheme.is_allowed(ModulationScheme::FSK));
  EXPECT_FALSE(modulation_scheme.is_allowed(ModulationScheme::LORA));
}

TEST(SystemDefinitionUsabilityTest, CanFindFunctionInSystemById) {
  EXPECT_EQ(modulation_scheme, *radio_1.get_function(as_int(RadioSettings::MODULATION_SCHEME)));
  EXPECT_EQ(line_coding, *radio_1.get_function(as_int(RadioSettings::LINE_CODING)));
  EXPECT_EQ(nullptr, radio_1.get_function(as_int(RadioSettings::TX_POWER)));
}

TEST(SystemDefinitionUsabilityTest, CanFindSystemsInSystemSimple) {
  EXPECT_EQ(radio_1,
            *transceiver_system.get_subsystem(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_1)));
}

TEST(SystemDefinitionUsabilityTest, CanFindSystemsInSystem) {
  ASSERT_NE(nullptr,
            transceiver_system.get_subsystem(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_1)))
      << to_string(transceiver_system);
  EXPECT_EQ(
      as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_1),
      transceiver_system.get_subsystem(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_1))->id());

  ASSERT_NE(nullptr,
            transceiver_system.get_subsystem(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_2)))
      << to_string(transceiver_system);
  EXPECT_EQ(
      as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_2),
      transceiver_system.get_subsystem(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_2))->id());
}

TEST(SystemDefinitionUsabilityTest, CanDefineComplexSystems) {
  EXPECT_EQ(as_int(Systems::NAVIGATION),
            satellite_42.get_subsystem(as_int(Systems::NAVIGATION))->id());
  EXPECT_EQ(as_int(Systems::POWER), satellite_42.get_subsystem(as_int(Systems::POWER))->id());
  EXPECT_EQ(as_int(Systems::COMMUNICATIONS),
            satellite_42.get_subsystem(as_int(Systems::COMMUNICATIONS))->id());
}

TEST(SystemDefinitionUsabilityTest, CanFindSubsystemsRecursively) {
  ASSERT_NE(nullptr, satellite_42.search_subsystems("2"));
  EXPECT_EQ(as_int(Systems::COMMUNICATIONS), satellite_42.search_subsystems("2")->id());

  ASSERT_NE(nullptr, satellite_42.search_subsystems("2.2"));
  EXPECT_EQ(as_int(CommunicationsSubsystems::TRANSCEIVER),
            satellite_42.search_subsystems("2.2")->id());
}

TEST(SystemDefinitionUsabilityTest, CanFindSystemsRecursively) {
  ASSERT_NE(nullptr, satellite_42.search_subsystems("2.2.0"));
  EXPECT_EQ(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_1),
            satellite_42.search_subsystems("2.2.0")->id());

  ASSERT_NE(nullptr, satellite_42.search_subsystems("2.2.1"));
  EXPECT_EQ(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_2),
            satellite_42.search_subsystems("2.2.1")->id());
}

TEST(SystemDefinitionUsabilityTest, SearchNonexistentSubsystemGivesNullptr) {
  EXPECT_EQ(nullptr, satellite_42.search_subsystems("2.2.3"));
}

TEST(SystemDefinitionUsabilityTest, CanGenerateString) {
  EXPECT_FALSE(to_string(satellite_42).empty());
}

}  // namespace tvsc::configuration
