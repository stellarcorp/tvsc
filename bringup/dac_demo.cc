#include "bringup/dac_demo.h"

#include <cstdint>

#include "base/initializer.h"
#include "system/system.h"

using BoardType = tvsc::system::System::BoardType;

__attribute__((section(".status.value"))) uint32_t dac1_value;
__attribute__((section(".status.value"))) uint32_t dac2_value;

using namespace tvsc::bringup;
using namespace tvsc::system;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  if constexpr (BoardType::NUM_DAC_CHANNELS >= 1) {
    System::scheduler().add_task(run_dac_demo<0>(dac1_value));
  }
  if constexpr (BoardType::NUM_DAC_CHANNELS >= 2) {
    System::scheduler().add_task(run_dac_demo<1>(dac2_value, 1250));
  }
  System::scheduler().start();
}
