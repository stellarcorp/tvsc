#include "hal/bringup/blink.h"

#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"

using BoardType = tvsc::hal::board::Board;

using namespace tvsc::hal::bringup;
using namespace tvsc::hal::scheduler;

static constexpr size_t QUEUE_SIZE{4};

int main() {
  BoardType board{};

  board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);

  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  auto& clock{board.clock()};

  Task task{blink(clock, gpio, BoardType::GREEN_LED_PIN)};

  Scheduler<QUEUE_SIZE> scheduler{clock};
  scheduler.add_task(task);

  scheduler.start();
}
