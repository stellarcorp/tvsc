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
  BoardType& board{BoardType::board()};

  auto& clock{board.clock()};

  Scheduler<QUEUE_SIZE> scheduler{clock};

  static constexpr uint64_t DURATION_MULTIPLES[] = {4, 3, 2};
  static_assert(BoardType::NUM_DEBUG_LEDS < 4, "Need to implement blink for more LEDs");

  for (size_t i = 0; i < BoardType::NUM_DEBUG_LEDS; ++i) {
    board.rcc().enable_gpio_port_clock(BoardType::DEBUG_LED_PORTS[i]);
    auto& gpio{board.gpio(BoardType::DEBUG_LED_PORTS[i])};
    scheduler.add_task(
        blink(clock, gpio, BoardType::DEBUG_LED_PINS[i], DURATION_MULTIPLES[i] * BASE_DURATION_MS));
  }

  scheduler.start();
}
