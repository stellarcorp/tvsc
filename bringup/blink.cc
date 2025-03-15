#include "bringup/blink.h"

#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "bringup/quit.h"
#include "hal/board/board.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

static constexpr size_t QUEUE_SIZE{2};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};

  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};
  scheduler.add_task(
      blink(clock, board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));
  scheduler.add_task(quit(scheduler));

  scheduler.start();
}
