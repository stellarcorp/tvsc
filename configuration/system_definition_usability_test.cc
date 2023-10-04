#include <iostream>

#include "configuration/allowed_values.h"
#include "configuration/system_definition.h"
#include "configuration/utility.h"
#include "gtest/gtest.h"

namespace tvsc::configuration {

enum class Systems : SystemId {
  NAVIGATION,
  POWER,
  COMMUNICATIONS,
};

enum class CommunicationsSubsystems : SystemId {
  LINK_MANAGEMENT,
  MULTIPLE_ACCESS,
  TRANSCEIVER,
};

enum class TransceiverSubsystems : SystemId {
  HALF_DUPLEX_RADIO,
  SIMPLEX_TX_RADIO,
  SIMPLEX_RX_RADIO,
  DUPLEX_RADIO,
  LOW_NOISE_AMPLIFIER,
  OSCILLATOR,
};

enum class RadioSettings : FunctionId {
  MODULATION_SCHEME,
  LINE_CODING,
  TX_POWER,
};

enum class ModulationScheme {
  CW,
  OOK,
  FSK,
  LORA,
};

enum class LineCoding {
  NONE,

  WHITENING,

  // This list primarily comes from:
  // https://en.wikipedia.org/wiki/Line_code#Transmission_and_storage
  NRZ_L,
  NRZ_M,
  NRZ_S,
  RZ,
  BIPHASE_L,
  BIPHASE_M,
  BIPHASE_S,

  // Manchester encoding and differential Manchester encoding both have multiple implementations.
  MANCHESTER_ORIGINAL,
  MANCHESTER_802_3,

  DIFFERENTIAL_MANCHESTER,

  BIPOLAR,
};

template <typename T>
constexpr T as_constant(T value) {
  return value;
}

// static constexpr auto allowed_modulation_schemes = {};

Function modulation_scheme{as_int(RadioSettings::MODULATION_SCHEME),
                           {ModulationScheme::FSK, ModulationScheme::OOK}};

Function line_coding{as_int(RadioSettings::LINE_CODING),
                     {LineCoding::NRZ_L, LineCoding::MANCHESTER_802_3}};

static constexpr ComponentId RADIO_1{1};
static CONSTEXPR_SETTINGS Component radio_1{RADIO_1,
                                            {
                                                modulation_scheme,
                                                line_coding,
                                            }};

TEST(SettingsUsabilityTest, CanTestAllowedValueForFunction) {
#if __cplusplus >= 202000
  // We expect to be able to check if particular values are allowed at compile time in C++20 and
  // later.
  static_assert(modulation_scheme.is_allowed(ModulationScheme::OOK));
  static_assert(modulation_scheme.is_allowed(ModulationScheme::FSK));
#endif

  EXPECT_TRUE(modulation_scheme.is_allowed(ModulationScheme::OOK));
  EXPECT_TRUE(modulation_scheme.is_allowed(ModulationScheme::FSK));
  EXPECT_FALSE(modulation_scheme.is_allowed(ModulationScheme::LORA));
}

TEST(SettingsUsabilityTest, CanFindFunctionInComponentById) {
#if __cplusplus >= 202000
  static_assert(*radio_1.search(RadioSettings::MODULATION_SCHEME) == modulation_scheme);
  static_assert(*radio_1.search(RadioSettings::LINE_CODING) == line_coding);
#endif

  EXPECT_EQ(modulation_scheme, *radio_1.search(as_int(RadioSettings::MODULATION_SCHEME)));
  EXPECT_EQ(line_coding, *radio_1.search(as_int(RadioSettings::LINE_CODING)));
  EXPECT_EQ(nullptr, radio_1.search(as_int(RadioSettings::TX_POWER)));
}

TEST(SettingsUsabilityTest, CanFindComponentsInSystem) {
  static CONSTEXPR_SETTINGS System trivial_transceiver_system{
      as_int(CommunicationsSubsystems::TRANSCEIVER),
      {radio_1},
  };

#if __cplusplus >= 202000
  static_assert(*trivial_transceiver_system.search_components(RADIO_1) == radio_1);
#endif

  EXPECT_EQ(radio_1, *trivial_transceiver_system.search_components(RADIO_1));
}

TEST(SettingsUsabilityTest, CanFindSubsystemsInSystem) {
  static CONSTEXPR_SETTINGS System full_transceiver_system{
      as_int(CommunicationsSubsystems::TRANSCEIVER),
      {
          {as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO), {radio_1}},
      },
  };

#if __cplusplus >= 202000
  static_assert(
      full_transceiver_system.search_subsystems(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO))
          ->identifier() == as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO));
#endif

  EXPECT_EQ(
      as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO),
      full_transceiver_system.search_subsystems(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO))
          ->identifier());
}

TEST(SettingsUsabilityTest, CanDefineComplexSystems) {
  static constexpr SystemId SATELLITE_ID{42};
  static CONSTEXPR_SETTINGS System satellite{
      SATELLITE_ID,
      {
          {as_int(Systems::NAVIGATION)},
          {as_int(Systems::POWER)},
          {
              as_int(Systems::COMMUNICATIONS),
              {
                  as_int(CommunicationsSubsystems::TRANSCEIVER),
                  {
                      as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO),
                      {radio_1},
                  },
              },
          },
      },
  };

#if __cplusplus >= 202000
  static_assert(satellite.search_subsystems(as_int(Systems::NAVIGATION))->identifier() ==
                as_int(Systems::NAVIGATION));
  static_assert(satellite.search_subsystems(as_int(Systems::POWER))->identifier() ==
                as_int(Systems::POWER));
  static_assert(satellite.search_subsystems(as_int(Systems::COMMUNICATIONS))->identifier() ==
                as_int(Systems::COMMUNICATIONS));
#endif

  EXPECT_EQ(as_int(Systems::NAVIGATION),
            satellite.search_subsystems(as_int(Systems::NAVIGATION))->identifier());
  EXPECT_EQ(as_int(Systems::POWER),
            satellite.search_subsystems(as_int(Systems::POWER))->identifier());
  EXPECT_EQ(as_int(Systems::COMMUNICATIONS),
            satellite.search_subsystems(as_int(Systems::COMMUNICATIONS))->identifier());
}

TEST(SettingsUsabilityTest, CanFindSubsystemsRecursively) {
  static constexpr SystemId SATELLITE_ID{42};
  static CONSTEXPR_SETTINGS System satellite{
      SATELLITE_ID,
      {
          System{as_int(Systems::NAVIGATION)},
          System{as_int(Systems::POWER)},
          System{
              as_int(Systems::COMMUNICATIONS),
              {
                  System{
                      as_int(CommunicationsSubsystems::TRANSCEIVER),
                      {
                          System{
                              as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO),
                              {radio_1},
                          },
                          System{as_int(TransceiverSubsystems::OSCILLATOR)},
                      },
                  },
              },
          },
      },
  };

  EXPECT_EQ(as_int(CommunicationsSubsystems::TRANSCEIVER),
            satellite.search_subsystems("2.2")->identifier());

  EXPECT_EQ(as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO),
            satellite.search_subsystems("2.2.0")->identifier());

  EXPECT_EQ(as_int(TransceiverSubsystems::OSCILLATOR),
            satellite.search_subsystems("2.2.5")->identifier());
}

}  // namespace tvsc::configuration
