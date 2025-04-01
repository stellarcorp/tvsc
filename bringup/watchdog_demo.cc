#include <chrono>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/quit.h"
#include "bringup/watchdog.h"
#include "hal/board/board.h"
#include "scheduler/scheduler.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  static constexpr auto CYCLE_TIME{5s};
  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};

  // Sleep now so that we can detect the board reset. If we just launch into the tasks, we might not
  // be able to detect that the watchdog caused a reset.
  clock.sleep(CYCLE_TIME);

  Scheduler<ClockType, /* QUEUE_SIZE */ 4> scheduler{board.rcc()};
  scheduler.add_task(run_watchdog<ClockType>(board.iwdg()));
  scheduler.add_task(
      blink(clock, board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));
  scheduler.add_task(quit(scheduler, CYCLE_TIME));
  scheduler.start();
}
