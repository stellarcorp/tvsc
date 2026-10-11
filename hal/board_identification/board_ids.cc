#include "hal/board_identification/board_ids.h"

#include <algorithm>
#include <array>
#include <cstdint>

#include "base/enums.h"
#include "hal/board_identification/voltage_divider_configurations.h"

namespace tvsc::hal::board_identification {

const std::array<VoltageDivider<BOARD_ID_ADC_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>,
                 NUM_BOARD_IDS>&
voltage_divider_configurations() {
  static const auto CONFIGURATIONS{
      generate_voltage_divider_configurations<NUM_BOARD_IDS, BOARD_ID_ADC_RESOLUTION_BITS,
                                              BOARD_ID_RESISTOR_TOLERANCE>(
          VOLTAGE_DIVIDER_TOTAL_RESISTANCE)};

  return CONFIGURATIONS;
}

BoardId determine_board_id(uint16_t adc_measurement) {
  BoardId result{cast_to_underlying_type(CanonicalBoardIds::UNKNOWN)};

  for (const auto& configuration : voltage_divider_configurations()) {
    if (configuration.is_match(adc_measurement)) {
      result = configuration.id();
      break;
    }
  }

  // Due to a mistake in the board design, the comms board in Cheerful Coffee has 221k ohm
  // resistors in the board identification voltage dividers. It should have had 22k1 ohm resistors
  // instead. The section of code here works around this issue and allows the boards to correctly
  // identify themselves.
#ifdef CHEERFUL_COFFEE
  if (result == cast_to_underlying_type(CanonicalBoardIds::UNKNOWN)) {
    static constexpr VoltageDivider<BOARD_ID_ADC_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>
        ERRANT_COMMS1_BOARD{221'000.f, 3'160.f};
    static constexpr VoltageDivider<BOARD_ID_ADC_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>
        ERRANT_COMMS2_BOARD{3'160.f, 221'000.f};

    if (ERRANT_COMMS1_BOARD.is_match(adc_measurement)) {
      result = cast_to_underlying_type(CanonicalBoardIds::COMMS_BOARD_1);
    } else if (ERRANT_COMMS2_BOARD.is_match(adc_measurement)) {
      result = cast_to_underlying_type(CanonicalBoardIds::COMMS_BOARD_2);
    }
  }
#endif

  return result;
}

}  // namespace tvsc::hal::board_identification
