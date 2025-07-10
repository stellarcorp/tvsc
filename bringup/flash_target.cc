#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "bits/bits.h"
#include "hal/board/board.h"
#include "hal/programmer/programmer.h"
#include "meta/build_time.h"
#include "meta/firmware.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "serial_wire/serial_wire.h"
#include "serial_wire/target.h"
#include "time/embedded_clock.h"

extern "C" {
__attribute__((section(".status.value"))) tvsc::meta::BuildTime target_build_time{};
__attribute__((section(".status.value"))) uint32_t result_dp_idcode{};
__attribute__((section(".status.value"))) uint32_t result_ap_idr{};
}

namespace tvsc::bringup {

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

template <typename ClockType>
tvsc::scheduler::Task<ClockType> flash_target(BoardType &board) {
  using namespace std::chrono_literals;
  using namespace tvsc::hal::gpio;
  using namespace tvsc::serial_wire;

  auto &debug_led_peripheral{board.gpio<BoardType::DEBUG_LED_PORT>()};
  auto &programmer_peripheral{board.programmer()};

  auto debug_led{debug_led_peripheral.access()};

  debug_led.set_pin_mode(BoardType::DEBUG_LED_PIN, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    {
      result_dp_idcode = 0;
      result_ap_idr = 0;

      tvsc::serial_wire::SerialWire swd{programmer_peripheral};
      tvsc::serial_wire::Target target{swd};
      bool success{true};

      if (success) {
        success = target.read_dp_idcode(result_dp_idcode);
      }

      if (success) {
        success = target.read_ap_idr(result_ap_idr);
      }

      // Write target address to TAR.
      // Write to AP register at 0x04.
      // if (success && !swd.swd_ap_write(0x04, tvsc::meta::BUILD_TIME_ADDR)) {
      //   success = false;
      //   step = 35;
      // }

      // // Read from DRW to trigger memory read.
      // if (success && !swd.swd_ap_read(0x0c, dummy)) {
      //   success = false;
      //   step = 40;
      // }

      // // // Read result.
      // if (success && !swd.swd_dp_read(0x00, target_build_time.timestamp)) {
      //   success = false;
      //   step = 50;
      // }

      // If we get a successful read of IDCODE, read the build time of the target.
      if (success) {
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

    co_yield 1s;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType &board{BoardType::board()};

  Scheduler<ClockType, 1 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(flash_target<ClockType>(board));
  scheduler.start();
}
