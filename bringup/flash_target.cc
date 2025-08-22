#include "bringup/flash_target.h"

#include "base/initializer.h"
#include "hal/board/board.h"
#include "system/scheduler.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::system;

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType &board{BoardType::board()};

  Scheduler<ClockType, 1 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(flash_target<ClockType>(
      board.programmer(), board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));
  scheduler.start();
}
