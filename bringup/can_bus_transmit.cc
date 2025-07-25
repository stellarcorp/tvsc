#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/can_bus/can_bus.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

extern "C" {
__attribute__((section(".status.value"))) volatile uint32_t identifier{};
__attribute__((section(".status.value"))) volatile uint32_t tx_count{};
__attribute__((section(".status.value"))) volatile uint32_t error_count{};
__attribute__((section(".status.value"))) volatile uint32_t error_code{};
}

namespace tvsc::bringup {

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

static constexpr char MESSAGE[] = "Hello";

template <typename ClockType>
tvsc::scheduler::Task<ClockType> echo_client(BoardType& board) {
  using namespace std::chrono_literals;
  using namespace tvsc::hal::can_bus;
  using namespace tvsc::hal::gpio;

  auto& debug_led_peripheral{board.gpio<BoardType::DEBUG_LED_PORT>()};
  auto& can1_peripheral{board.can1()};

  // Turn on clocks for the peripherals that we want.
  auto debug_led{debug_led_peripheral.access()};
  auto can1{can1_peripheral.access()};

  error_code = can1.error_code();
  while (error_code != 0) {
  }

  debug_led.set_pin_mode(BoardType::DEBUG_LED_PIN, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    using std::to_string;
    if (can1.transmit(++identifier, MESSAGE)) {
      ++tx_count;
      debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
      co_yield 2ms;
      debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
    } else {
      ++error_count;
      error_code = can1.error_code();
      co_yield 2ms;
    }

    co_yield 250ms;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType& board{BoardType::board()};

  Scheduler<ClockType, 1 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(echo_client<ClockType>(board));
  scheduler.start();
}
