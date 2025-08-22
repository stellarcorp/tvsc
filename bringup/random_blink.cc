#include "bringup/random_blink.h"

#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "system/scheduler.h"
#include "system/task.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::system;

static constexpr size_t QUEUE_SIZE{1};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType& board{BoardType::board()};

  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};
  scheduler.add_task(
      blink_randomly<ClockType>(board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));

  scheduler.start();
}
