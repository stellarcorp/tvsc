#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/flash_target.h"
#include "hal/board/board.h"
#include "scheduler/scheduler.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};

  static constexpr size_t QUEUE_SIZE{4};
  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};
  scheduler.add_task(flash_target<ClockType>(
      board.programmer(), board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));
  scheduler.add_task(
      blink(clock, board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));
  scheduler.start();
}
