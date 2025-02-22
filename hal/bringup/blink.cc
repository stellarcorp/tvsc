#include "hal/bringup/blink.h"

#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = BoardType::ClockType;
using TaskType = tvsc::hal::scheduler::Task<ClockType>;

using namespace tvsc::hal::bringup;
using namespace tvsc::hal::scheduler;

static constexpr size_t QUEUE_SIZE{1};

int main() {
  BoardType& board{BoardType::board()};
  auto& clock{board.clock()};

  Scheduler<ClockType, QUEUE_SIZE> scheduler{clock, board.rcc()};
  scheduler.add_task(
      blink(clock, board.gpio<BoardType::GREEN_LED_PORT>(), BoardType::GREEN_LED_PIN));

  scheduler.start();
}
