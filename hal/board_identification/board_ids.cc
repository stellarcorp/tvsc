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
    if (configuration.min_adc_measurement_value() <= adc_measurement &&
        adc_measurement <= configuration.max_adc_measurement_value()) {
      result = configuration.id();
      break;
    }
  }
  return result;
}

}  // namespace tvsc::hal::board_identification
