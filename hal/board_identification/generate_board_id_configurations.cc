#include <iomanip>
#include <iostream>

#include "base/initializer.h"
#include "hal/board_identification/board_ids.h"

using namespace tvsc::hal::board_identification;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  for (const auto& configuration : voltage_divider_configurations()) {
    if (configuration.id() != 0) {
      std::cout << std::showbase << std::hex << static_cast<int>(configuration.id()) << " ("
                << std::dec << static_cast<int>(configuration.id()) << ") -- "
                << configuration.high_resistor_value() << "Ω, "
                << configuration.low_resistor_value() << "Ω (adc measurement range: [" << std::hex
                << configuration.min_adc_measurement_value() << ", "
                << configuration.max_adc_measurement_value()
                << "], source impedance: " << configuration.source_impedance() << "Ω)"
                << "\n";
    }
  }

  return 0;
}
