#include <cstdint>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/dma/channel.h"
#include "hal/i2c/i2c.h"
#include "platforms/linker_sections.h"

struct alignas(16) ImuIds final {
  uint16_t imu1_id{};
  uint16_t imu2_id{};
  bool imu1_read_success{};
  bool imu2_read_success{};
};

PRINCIPAL_RESULT ImuIds ids{};

using BoardType = tvsc::hal::board::Board;
using namespace tvsc::hal::dma;

struct DmaConfiguration final : DefaultDmaConfiguration {
  // TODO(james): Add binding for SPI bus DMAs.
};

DEFINE_DMA_IRQ_HANDLERS(DmaConfiguration);

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  initialize_dma_interrupts<DmaConfiguration>();

  BoardType& board{BoardType::board()};
  auto& imu1_peripheral{board.imu<0>()};
  auto& imu2_peripheral{board.imu<1>()};

  auto imu1{imu1_peripheral.access()};
  auto imu2{imu2_peripheral.access()};

  while (true) {
    ids.imu1_read_success = imu1.read_id(ids.imu1_id);
    ids.imu2_read_success = imu2.read_id(ids.imu2_id);
  }
}
