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
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "serial_wire/flash.h"
#include "serial_wire/serial_wire.h"
#include "serial_wire/target.h"
#include "time/embedded_clock.h"

static constexpr uint32_t READ_BASE_ADDRESS{0x0800'0000};
static constexpr uint32_t NUM_PAGES{1};
static constexpr size_t FLASH_PAGE_SIZE_BYTES{2048};

extern "C" {
__attribute__((section(".status.value"))) tvsc::meta::BuildTime target_build_time{};
__attribute__((section(".status.value"))) uint32_t result_dp_idr{};
__attribute__((section(".status.value"))) uint32_t result_ap_idr{};
__attribute__((section(".status.value"))) std::array<uint32_t, 16> target_mem{};
__attribute__((section(".status.value"))) uint32_t ctrl_stat_errors{};
__attribute__((section(".status.value"))) uint32_t first_different_page{};
__attribute__((section(".status.value")))
std::array<uint32_t, NUM_PAGES * FLASH_PAGE_SIZE_BYTES / sizeof(uint32_t)>
    read_back{};
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

    // if (success) {
    //   success = target.enable_debug();
    // }

    // if (success) {
    //   success = target.ap_read_mem(tvsc::meta::BUILD_TIME_ADDR,
    //                                reinterpret_cast<uint32_t *>(&target_build_time),
    //                                sizeof(tvsc::meta::BuildTime) / sizeof(uint32_t));
    // }

    std::array<uint32_t, NUM_PAGES * FLASH_PAGE_SIZE_BYTES / sizeof(uint32_t)> test_pages{};
    for (size_t i = 0; i < test_pages.size(); ++i) {
      test_pages[i] = i;
    }
    if (success) {
      debug_led.write_pin(BoardType::DEBUG_LED_PIN, 1);
      success =
          flash.write_flash(0x08000000, tvsc::meta::firmware_start, tvsc::meta::firmware_size);
      // success = flash.write_flash(0x08000000, &test_pages[0], test_pages.size());
      debug_led.write_pin(BoardType::DEBUG_LED_PIN, 0);
    }

    // if (success) {
    //   success = target.disable_debug();
    // }

    if (success) {
      for (first_different_page = 0;
           success && first_different_page <
                          (tvsc::meta::firmware_size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
           ++first_different_page) {
        success = target.ap_read_mem(0x08000000 + first_different_page * FLASH_PAGE_SIZE,
                                     &read_back[0], read_back.size());
        if (success) {
          for (size_t i = 0; i < read_back.size(); ++i) {
            if (read_back[i] != *(reinterpret_cast<const uint32_t *>(0x08000000) +
                                  first_different_page * FLASH_PAGE_SIZE / sizeof(uint32_t) + i)) {
              break;
            }
          }
        }
      }
    }

    //(void)target.power_off();

    swd.reset_target();
  }

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
