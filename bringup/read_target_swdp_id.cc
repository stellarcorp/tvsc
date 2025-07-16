#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "bits/bits.h"
#include "hal/board/board.h"
#include "hal/programmer/programmer.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "serial_wire/serial_wire.h"
#include "serial_wire/target.h"
#include "time/embedded_clock.h"

extern "C" {
__attribute__((section(".status.value"))) uint32_t target_swdp_id{};
}

namespace tvsc::bringup {

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

template <typename ClockType>
tvsc::scheduler::Task<ClockType> read_target_swdp_id(BoardType &board) {
  using namespace std::chrono_literals;
  using namespace tvsc::hal::gpio;
  using namespace tvsc::serial_wire;

  static constexpr uint32_t EXPECTED_SW_DP_IDCODE{0x2BA01477};

  auto &debug_led_peripheral{board.gpio<BoardType::DEBUG_LED_PORT>()};
  auto &programmer_peripheral{board.programmer()};

  // Turn on clocks for the peripherals that we want.
  auto debug_led{debug_led_peripheral.access()};

  debug_led.set_pin_mode(BoardType::DEBUG_LED_PIN, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    {
      target_swdp_id = 0;

      tvsc::serial_wire::SerialWire swd{programmer_peripheral};
      tvsc::serial_wire::Target target{swd};

      tvsc::serial_wire::Result success{};
      success = target.read_idr(target_swdp_id);

      // Successful read.
      if (success && target_swdp_id == EXPECTED_SW_DP_IDCODE) {
        for (int i = 0; i < 5; ++i) {
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
          co_yield 250ms;
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
          co_yield 250ms;
        }
      } else {
        for (int i = 0; i < 5; ++i) {
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
          co_yield 50ms;
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
          co_yield 50ms;
        }
      }
    }

    co_yield 2s;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType &board{BoardType::board()};

  Scheduler<ClockType, 1 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(read_target_swdp_id<ClockType>(board));
  scheduler.start();
}
