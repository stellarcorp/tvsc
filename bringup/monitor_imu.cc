#include "bringup/monitor_imu.h"

#include <chrono>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/watchdog.h"
#include "hal/dma/channel.h"
#include "system/system.h"

using namespace tvsc::bringup;
using namespace tvsc::system;
using namespace std::chrono_literals;

extern "C" {

__attribute__((section(".status.value"))) ImuReading imu1_reading{};
__attribute__((section(".status.value"))) ImuReading imu2_reading{};

}  // extern "C"

using namespace tvsc::hal::dma;

struct DmaConfiguration final : DefaultDmaConfiguration {
  // TODO(james): Add binding for SPI bus DMAs.
};

DEFINE_DMA_IRQ_HANDLERS(DmaConfiguration);

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  initialize_dma_interrupts<DmaConfiguration>();

  auto& mcu{System::mcu()};
  auto& board{System::board()};
  auto& scheduler{System::scheduler()};

  scheduler.add_task(monitor_imu(board.imu<0>(), imu1_reading, 1ms));
  // scheduler.add_task(monitor_imu<ClockType>(board.imu<1>(), imu2_reading,
  // 1ms));
  scheduler.add_task(blink(board.debug_led()));
  scheduler.add_task(run_watchdog(mcu.iwdg()));

  scheduler.start();
}
