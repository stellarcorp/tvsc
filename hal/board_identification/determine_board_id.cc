#include <iostream>
#include <optional>

#include "base/initializer.h"
#include "gflags/gflags.h"
#include "hal/board_identification/board_ids.h"

using namespace tvsc::hal::board_identification;

DEFINE_int32(adc_measurement, 0, "Value measured by ADC");

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  std::optional<BoardId> id{determine_board_id(static_cast<uint16_t>(FLAGS_adc_measurement))};

  if (id) {
    std::cout << "Board identified as board '" << static_cast<int>(*id) << "'.\n";
  } else {
    std::cout << "Board not identified.\n";
  }

  return 0;
}
