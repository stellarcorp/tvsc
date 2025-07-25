#include <chrono>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/programmer/programmer.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

namespace tvsc::bringup {

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

template <typename ClockType>
tvsc::scheduler::Task<ClockType> reset_target(BoardType &board) {
  using namespace std::chrono_literals;
  using namespace tvsc::hal::gpio;

  auto &debug_led_peripheral{board.gpio<BoardType::DEBUG_LED_PORT>()};
  auto &programmer_peripheral{board.programmer()};

  // Turn on clocks for the peripherals that we want.
  auto debug_led{debug_led_peripheral.access()};

  debug_led.set_pin_mode(BoardType::DEBUG_LED_PIN, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    co_yield 2s;

    {
      for (int i = 0; i < 5; ++i) {
        debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
        co_yield 25ms;
        debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
        co_yield 25ms;
      }

      auto programmer{programmer_peripheral.access()};
      programmer.initiate_target_board_reset();
      // The following value should be sufficient to cause a reset of the target board.
      co_yield programmer.RESET_HOLD_TIME;
      // But we add this extra delay to make the effect more obvious.
      co_yield 750ms;
      programmer.conclude_target_board_reset();
    }

    co_yield 3s;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType &board{BoardType::board()};

  Scheduler<ClockType, 1 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(reset_target<ClockType>(board));
  scheduler.start();
}
