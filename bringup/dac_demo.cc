#include "bringup/dac_demo.h"

#include <cstddef>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "scheduler/scheduler.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

static constexpr size_t QUEUE_SIZE{BoardType::NUM_DAC_CHANNELS};

__attribute__((section(".status.value"))) uint32_t dac1_value;
__attribute__((section(".status.value"))) uint32_t dac2_value;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{BoardType::board()};

  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};

  if constexpr (BoardType::NUM_DAC_CHANNELS >= 1) {
    scheduler.add_task(run_dac_demo<ClockType, 0>(board, dac1_value));
  }
  if constexpr (BoardType::NUM_DAC_CHANNELS >= 2) {
    scheduler.add_task(run_dac_demo<ClockType, 1>(board, dac2_value, 1250));
  }
  scheduler.start();
}
