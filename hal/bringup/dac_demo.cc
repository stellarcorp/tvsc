#include "hal/bringup/dac_demo.h"

#include <cstddef>

#include "hal/board/board.h"
#include "hal/scheduler/scheduler.h"

using BoardType = tvsc::hal::board::Board;
using namespace tvsc::hal::bringup;
using namespace tvsc::hal::scheduler;

static constexpr size_t QUEUE_SIZE{BoardType::NUM_DAC_CHANNELS};

__attribute__((section(".status.value"))) uint32_t dac1_value;
__attribute__((section(".status.value"))) uint32_t dac2_value;

int main() {
  BoardType& board{BoardType::board()};

  Scheduler<QUEUE_SIZE> scheduler{board.clock()};

  if constexpr (BoardType::NUM_DAC_CHANNELS >= 1) {
    scheduler.add_task(run_dac_demo<0>(board, dac1_value));
  }
  if constexpr (BoardType::NUM_DAC_CHANNELS >= 2) {
    scheduler.add_task(run_dac_demo<1>(board, dac2_value, 1250));
  }
  scheduler.start();
}
