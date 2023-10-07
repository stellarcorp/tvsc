#pragma once

#include "configuration/system_definition.h"
#include "configuration/types.h"

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
  HALF_DUPLEX_RADIO_1,
  HALF_DUPLEX_RADIO_2,
  OSCILLATOR,
};

enum class RadioSettings : FunctionId {
  MODULATION_SCHEME,
  LINE_CODING,
  TX_POWER,
  OSCILLATOR_FREQUENCY,
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

enum class HalfDuplexRadioProperties : uint8_t {
  RESET_PIN,
  INTERRUPT_PIN,
};

static AllowedValues modulation_scheme_values{ModulationScheme::OOK, ModulationScheme::FSK};
static Function modulation_scheme{as_int(RadioSettings::MODULATION_SCHEME),
                                  "modulation_scheme",
                                  modulation_scheme_values};

static Function line_coding{as_int(RadioSettings::LINE_CODING),
                            "line_coding",
                            {LineCoding::NRZ_L, LineCoding::MANCHESTER_802_3}};

static System radio_1{as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_1),
                      "radio_1",
                      {
			Property{as_int(HalfDuplexRadioProperties::RESET_PIN), "reset_pin", 2},
			Property{as_int(HalfDuplexRadioProperties::INTERRUPT_PIN), "interrupt_pin", 14},
		      },
                      {
                          modulation_scheme,
                          line_coding,
                      }};

static System radio_2{as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_2),
                      "radio_2",
                      {
			Property{as_int(HalfDuplexRadioProperties::RESET_PIN), "reset_pin", 6},
			Property{as_int(HalfDuplexRadioProperties::INTERRUPT_PIN), "interrupt_pin", 17},
		      },
                      {
                          modulation_scheme,
                          line_coding,
                      }};

static System transceiver_system{
    as_int(CommunicationsSubsystems::TRANSCEIVER),
    "transceiver",
    {radio_1, radio_2},
};

static System advanced_transceiver_system{
    as_int(CommunicationsSubsystems::TRANSCEIVER),
    "transceiver",
    {
        radio_1,
        radio_2,
        System{
            as_int(TransceiverSubsystems::OSCILLATOR),
            "oscillator",
            {},
            {
                Function{
                    as_int(RadioSettings::OSCILLATOR_FREQUENCY),
                    "oscillator_frequency",
                    {
                        ValueRange<float>{410'000'000.f, 495'000'000.f},
                    }  //
                },
            },
        },
    },
};

static constexpr SystemId SATELLITE_ID{42};
static System satellite_42{
    SATELLITE_ID,
    "satellite_42",
    {
        System{as_int(Systems::NAVIGATION), "nav"},
        System{as_int(Systems::POWER), "power"},
        System{
            as_int(Systems::COMMUNICATIONS),
            "comms",
            {
                advanced_transceiver_system,
            },
        },
    },
};

}  // namespace tvsc::configuration
