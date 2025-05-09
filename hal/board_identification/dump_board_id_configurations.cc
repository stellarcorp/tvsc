#include <iostream>

#include "base/initializer.h"
#include "hal/board_identification/board_resistor_configurations.h"

using namespace tvsc::hal::board_identification;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  for (const auto& configuration : BOARD_ID_VOLTAGE_DIVIDER_CONFIGURATIONS) {
    std::cout << static_cast<int>(configuration.id()) << " -- "
              << configuration.high_resistor_value() << ", " << configuration.low_resistor_value()
              << "\n";
  }

  return 0;
}
