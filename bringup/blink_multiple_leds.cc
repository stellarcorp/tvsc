#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

static constexpr size_t QUEUE_SIZE{4};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  static constexpr std::chrono::milliseconds BASE_DURATION_MS{200};
  auto& board{BoardType::board()};

  auto& clock{ClockType::clock()};

  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};

  static constexpr int DURATION_MULTIPLES[] = {4, 3, 2};
  static_assert(BoardType::NUM_DEBUG_LEDS < 4, "Need to implement blink for more LEDs");

  for (size_t i = 0; i < BoardType::NUM_DEBUG_LEDS; ++i) {
    auto& gpio{board.gpio(BoardType::DEBUG_LED_PORTS[i])};
    scheduler.add_task(
        blink(clock, gpio, BoardType::DEBUG_LED_PINS[i], DURATION_MULTIPLES[i] * BASE_DURATION_MS));
  }

  scheduler.start();
}
