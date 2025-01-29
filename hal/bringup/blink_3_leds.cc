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

  static constexpr uint64_t DURATION_MULTIPLES[] = {4, 3, 2};

  if constexpr (BoardType::NUM_DEBUG_LEDS >= 1) {
    board.rcc().enable_gpio_port_clock(BoardType::DEBUG_LED_PORTS[0]);
    auto& gpio{board.gpio<BoardType::DEBUG_LED_PORTS[0]>()};
    scheduler.add_task(
        blink<DURATION_MULTIPLES[0] * BASE_DURATION_MS>(clock, gpio, BoardType::DEBUG_LED_PINS[0]));
  }

  if constexpr (BoardType::NUM_DEBUG_LEDS >= 2) {
    board.rcc().enable_gpio_port_clock(BoardType::DEBUG_LED_PORTS[1]);
    auto& gpio{board.gpio<BoardType::DEBUG_LED_PORTS[1]>()};
    scheduler.add_task(
        blink<DURATION_MULTIPLES[1] * BASE_DURATION_MS>(clock, gpio, BoardType::DEBUG_LED_PINS[1]));
  }

  if constexpr (BoardType::NUM_DEBUG_LEDS >= 3) {
    board.rcc().enable_gpio_port_clock(BoardType::DEBUG_LED_PORTS[2]);
    auto& gpio{board.gpio<BoardType::DEBUG_LED_PORTS[2]>()};
    scheduler.add_task(
        blink<DURATION_MULTIPLES[2] * BASE_DURATION_MS>(clock, gpio, BoardType::DEBUG_LED_PINS[2]));
  }

  static_assert(BoardType::NUM_DEBUG_LEDS < 4, "Need to implement blink for more LEDs");

  scheduler.start();
}
