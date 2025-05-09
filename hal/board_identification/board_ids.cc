#include "hal/board_identification/board_ids.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include "hal/board_identification/voltage_divider_configurations.h"

namespace tvsc::hal::board_identification {

std::vector<VoltageDivider<BOARD_ID_ADC_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>>
voltage_divider_configurations() {
  using VoltageDividerType =
      VoltageDivider<BOARD_ID_ADC_RESOLUTION_BITS, BOARD_ID_RESISTOR_TOLERANCE>;

  static const auto CONFIGURATIONS{
      generate_voltage_divider_configurations<NUM_BOARD_IDS, BOARD_ID_ADC_RESOLUTION_BITS,
                                              BOARD_ID_RESISTOR_TOLERANCE>(
          VOLTAGE_DIVIDER_TOTAL_RESISTANCE)};

  auto end_pos = std::find_if(CONFIGURATIONS.begin(), CONFIGURATIONS.end(),
                              [](const VoltageDividerType& divider) { return divider.id() == 0; });

  return std::vector<VoltageDividerType>{CONFIGURATIONS.begin(), end_pos};
}

std::optional<BoardId> determine_board_id(uint16_t adc_measurement) {
  std::optional<BoardId> result{};
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
