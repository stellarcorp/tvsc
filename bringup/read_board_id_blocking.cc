#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "bringup/read_board_id.h"
#include "system/system.h"

using namespace std::chrono_literals;
using namespace tvsc::hal::board_identification;

extern "C" {
alignas(uint32_t)  //
    __attribute__((section(".status.value"))) volatile BoardId board_id{};
}

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  using Pinout = tvsc::system::System::PinoutType;

  auto& system{tvsc::system::System::get()};
  auto& mcu{system.mcu()};
  auto& clock{system.clock()};

  auto id_power_peripheral{mcu.create_peripheral(Pinout::BOARD_ID_POWER_PIN)};
  auto id_sense_peripheral{mcu.create_peripheral(Pinout::BOARD_ID_SENSE_PIN)};
  auto& adc_peripheral{mcu.adc()};

  while (true) {
    board_id = tvsc::bringup::read_board_id(clock, std::move(id_power_peripheral),
                                            std::move(id_sense_peripheral), adc_peripheral);

    system.clock().wait(10ms);
  }
}
