#include "bringup/flash_target.h"

#include "base/initializer.h"
#include "hal/dma/channel.h"
#include "system/system.h"

using namespace tvsc::bringup;
using namespace tvsc::system;
using namespace tvsc::hal::dma;

DEFINE_DMA_IRQ_HANDLERS(DefaultDmaConfiguration);

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);
  initialize_dma_interrupts<DefaultDmaConfiguration>();

  auto &board{System::board()};

  System::scheduler().add_task(flash_target(board.programmer(), board.debug_led()));
  System::scheduler().start();
}
