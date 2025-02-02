#include "hal/bringup/blink.h"

#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/power_token.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"

using BoardType = tvsc::hal::board::Board;

using namespace tvsc::hal::bringup;
using namespace tvsc::hal::scheduler;

static constexpr size_t QUEUE_SIZE{1};

int main() {
  BoardType& board{BoardType::board()};

  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  tvsc::hal::PowerToken gpio_power{gpio.enable()};

  auto& clock{board.clock()};

  Scheduler<QUEUE_SIZE> scheduler{clock};
  scheduler.add_task(blink(clock, gpio, BoardType::GREEN_LED_PIN));

  scheduler.start();
}
