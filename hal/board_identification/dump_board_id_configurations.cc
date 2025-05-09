#include <iostream>

#include "base/initializer.h"
#include "hal/board_identification/board_ids.h"
#include "hal/board_identification/board_resistor_configurations.h"

using namespace tvsc::hal::board_identification;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  constexpr auto BOARD_ID_VOLTAGE_DIVIDER_CONFIGURATIONS =
      generate_voltage_divider_configurations<NUM_BOARD_IDS, BOARD_ID_ADC_RESOLUTION_BITS,
                                              BOARD_ID_RESOLUTION_BITS,
                                              BOARD_ID_RESISTOR_TOLERANCE>(
          VOLTAGE_DIVIDER_TOTAL_RESISTANCE);

  for (const auto& configuration : BOARD_ID_VOLTAGE_DIVIDER_CONFIGURATIONS) {
    std::cout << static_cast<int>(configuration.id()) << " -- "
              << configuration.high_resistor_value() << ", " << configuration.low_resistor_value()
              << " (adc measurement range: [" << configuration.min_adc_measurement_value() << ", "
              << configuration.max_adc_measurement_value() << "]"
              << "\n";
  }

  return 0;
}
