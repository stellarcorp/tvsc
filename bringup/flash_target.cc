#include <array>
#include <chrono>
#include <cstdint>
#include <span>

#include "base/initializer.h"
#include "bits/bits.h"
#include "hal/board/board.h"
#include "hal/programmer/programmer.h"
#include "meta/build_time.h"
#include "meta/firmware.h"
#include "meta/flash.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "serial_wire/flash.h"
#include "serial_wire/serial_wire.h"
#include "serial_wire/target.h"
#include "time/embedded_clock.h"

static constexpr uint32_t NUM_PAGES{2};
static constexpr uint32_t WRITE_BASE_ADDRESS{0x0800'0000};

static constexpr uint8_t FLASH_WRITE_START_PAGE{
    (WRITE_BASE_ADDRESS - 0x0800'0000 + tvsc::meta::FLASH_PAGE_SIZE_BYTES - 1) /
    tvsc::meta::FLASH_PAGE_SIZE_BYTES};

extern "C" {
__attribute__((section(".status.value"))) tvsc::meta::BuildTime target_build_time{};
__attribute__((section(".status.value"))) uint32_t result_dp_idr{};
__attribute__((section(".status.value"))) uint32_t result_ap_idr{};
__attribute__((section(".status.value"))) uint32_t ctrl_stat_errors{};
__attribute__((section(".status.value"))) bool target_initialized{};
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

  co_yield 50ms;

  for (int i = 0; i < 3; ++i) {
    debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
    co_yield 250ms;
    debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
    co_yield 250ms;
  }

  tvsc::serial_wire::Result success{};
  {
    result_dp_idr = 0;
    result_ap_idr = 0;

    tvsc::serial_wire::SerialWire swd{programmer_peripheral};
    tvsc::serial_wire::Target target{swd};
    target_initialized = target.is_initialized();

    if (target_initialized) {
      tvsc::serial_wire::Flash flash{target};

      if (success) {
        success = target.read_idr(result_dp_idr);
      }

      if (success) {
        success = target.initialize_ap_connection(0);
      }

      if (success) {
        success = target.read_ap_id(result_ap_idr);
      }

      if (success) {
        success = target.ap_read_mem(tvsc::meta::BUILD_TIME_ADDR,
                                     reinterpret_cast<uint32_t *>(&target_build_time),
                                     sizeof(tvsc::meta::BuildTime) / sizeof(uint32_t));
      }

      if (success && target_build_time.timestamp < tvsc::meta::BUILD_TIME.timestamp) {
        if (success) {
          success = target.halt();
        }

        if (success) {
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
          success = flash.write_pages(FLASH_WRITE_START_PAGE, tvsc::meta::firmware_size_pages,
                                      tvsc::meta::firmware_start);
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
        }

        (void)target.disable_debug();
        swd.reset_target();

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

      } else {
        for (int i = 0; i < 5; ++i) {
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
          co_yield 50ms;
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
          co_yield 50ms;
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
          co_yield 250ms;
          debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
          co_yield 250ms;
        }
      }
    }
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
