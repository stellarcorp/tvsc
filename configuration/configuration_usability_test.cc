#include <iostream>

#include "configuration/allowed_values.h"
#include "configuration/configuration.h"
#include "configuration/fake_systems.h"
#include "configuration/system_definition.h"
#include "configuration/types.h"
#include "gtest/gtest.h"

namespace tvsc::configuration {

static SystemConfiguration satellite_42_configuration_1{
    SATELLITE_ID,
    {
        SystemConfiguration{as_int(Systems::NAVIGATION)},
        SystemConfiguration{as_int(Systems::POWER)},
        SystemConfiguration{
            as_int(Systems::COMMUNICATIONS),
            {
                SystemConfiguration{
                    as_int(CommunicationsSubsystems::TRANSCEIVER),
                    {
                        SystemConfiguration{
                            as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_1),
                            {
                                Setting{as_int(RadioSettings::MODULATION_SCHEME),
                                        as_int(ModulationScheme::FSK)},
                                Setting{as_int(RadioSettings::LINE_CODING),
                                        as_int(LineCoding::MANCHESTER_ORIGINAL)},
                            },
                        },
                        SystemConfiguration{
                            as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_2),
                            {
                                Setting{as_int(RadioSettings::MODULATION_SCHEME),
                                        as_int(ModulationScheme::FSK)},
                                Setting{as_int(RadioSettings::LINE_CODING),
                                        as_int(LineCoding::MANCHESTER_ORIGINAL)},
                            },
                        },
                    },
                },
            },
        },

    },
};

static SystemConfiguration satellite_42_configuration_2{
    SATELLITE_ID,
    {
        SystemConfiguration{as_int(Systems::NAVIGATION)},
        SystemConfiguration{as_int(Systems::POWER)},
        SystemConfiguration{
            as_int(Systems::COMMUNICATIONS),
            {
                SystemConfiguration{
                    as_int(CommunicationsSubsystems::TRANSCEIVER),
                    {
                        SystemConfiguration{
                            as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_1),
                            {
                                Setting{as_int(RadioSettings::MODULATION_SCHEME),
                                        as_int(ModulationScheme::FSK)},
                                Setting{as_int(RadioSettings::LINE_CODING),
                                        as_int(LineCoding::WHITENING)},
                            },
                        },
                        SystemConfiguration{
                            as_int(TransceiverSubsystems::HALF_DUPLEX_RADIO_2),
                            {
                                Setting{as_int(RadioSettings::MODULATION_SCHEME),
                                        as_int(ModulationScheme::OOK)},
                                Setting{as_int(RadioSettings::LINE_CODING),
                                        as_int(LineCoding::NRZ_L)},
                            },
                        },
                    },
                },
            },
        },

    },
};

TEST(SystemConfigurationUsabilityTest, CanGenerateString) {
  EXPECT_FALSE(to_string(satellite_42_configuration_1).empty());
}

}  // namespace tvsc::configuration
