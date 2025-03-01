#include "hal/bringup/blink.h"

#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::hal::time::EmbeddedClock;

using namespace tvsc::hal::bringup;
using namespace tvsc::hal::scheduler;

static constexpr size_t QUEUE_SIZE{1};

int main() {
  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};

  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};
  scheduler.add_task(
      blink(clock, board.gpio<BoardType::GREEN_LED_PORT>(), BoardType::GREEN_LED_PIN));

  scheduler.start();
}
