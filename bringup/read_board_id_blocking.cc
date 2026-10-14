#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "bringup/read_board_id.h"
#include "hal/dma/channel.h"
#include "platforms/linker_sections.h"
#include "system/system.h"

using namespace std::chrono_literals;
using namespace tvsc::hal::board_identification;
using namespace tvsc::hal::dma;

inline PRINCIPAL_RESULT BoardId board_id{};

struct DmaConfiguration final : DefaultDmaConfiguration {
  using Dma1Channel1Facility = AdcFacility<0>;
};
DEFINE_DMA_IRQ_HANDLERS(DmaConfiguration);

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  initialize_dma_interrupts<DmaConfiguration>();

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

    system.clock().wait(5s);
  }
}
