#include "bringup/random_blink.h"

#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::hal::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::hal::scheduler;

static constexpr size_t QUEUE_SIZE{1};

int main() {
  BoardType& board{BoardType::board()};

  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};
  scheduler.add_task(
      blink_randomly<ClockType>(board.gpio<BoardType::GREEN_LED_PORT>(), BoardType::GREEN_LED_PIN));

  scheduler.start();
}
