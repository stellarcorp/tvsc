#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/bringup/blink.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"

using BoardType = tvsc::hal::board::Board;

using namespace tvsc::hal::bringup;
using namespace tvsc::hal::scheduler;

static constexpr size_t QUEUE_SIZE{4};

int main() {
  static constexpr uint64_t BASE_DURATION_MS{200};
  BoardType board{};

  auto& clock{board.clock()};

  Scheduler<QUEUE_SIZE> scheduler{clock};

  board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);
  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  scheduler.add_task(blink<4 * BASE_DURATION_MS>(clock, gpio, BoardType::GREEN_LED_PIN));

  board.rcc().enable_gpio_port_clock(BoardType::YELLOW_LED_PORT);
  auto& gpio2{board.gpio<BoardType::YELLOW_LED_PORT>()};
  scheduler.add_task(blink<3 * BASE_DURATION_MS>(clock, gpio2, BoardType::YELLOW_LED_PIN));

  board.rcc().enable_gpio_port_clock(BoardType::RED_LED_PORT);
  auto& gpio3{board.gpio<BoardType::RED_LED_PORT>()};
  scheduler.add_task(blink<2 * BASE_DURATION_MS>(clock, gpio3, BoardType::RED_LED_PIN));

  scheduler.start();
}
