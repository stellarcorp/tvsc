#include <chrono>
#include <cstring>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "hal/dma/channel.h"
#include "system/system.h"

using Pinout = tvsc::system::System::PinoutType;
using BoardType = tvsc::system::System::BoardType;

using namespace tvsc::bringup;
using namespace tvsc::system;
using namespace std::chrono_literals;
using namespace tvsc::hal::dma;

DEFINE_DMA_IRQ_HANDLERS(DefaultDmaConfiguration);

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  initialize_dma_interrupts<DefaultDmaConfiguration>();

  static constexpr auto BASE_DURATION{200ms};
  static constexpr int DURATION_MULTIPLES[] = {4, 3, 2};
  static_assert(Pinout::NUM_LEDS < 4, "Need to implement blink for more LEDs");

  for (size_t i = 0; i < Pinout::NUM_LEDS; ++i) {
    System::scheduler().add_task(
        blink(System::board().led(i), DURATION_MULTIPLES[i] * BASE_DURATION));
  }

  System::scheduler().start();
}
