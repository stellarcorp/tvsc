#include <chrono>
#include <cstdint>

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

static constexpr uint32_t NUM_PAGES{2};
static constexpr uint32_t WRITE_BASE_ADDRESS{0x0800'0000};

static constexpr uint8_t FLASH_WRITE_START_PAGE{
    (WRITE_BASE_ADDRESS - 0x0800'0000 + tvsc::meta::FLASH_PAGE_SIZE_BYTES - 1) /
    tvsc::meta::FLASH_PAGE_SIZE_BYTES};

extern "C" {
__attribute__((section(".status.value"))) tvsc::meta::BuildTime target_build_time{};
__attribute__((section(".status.value"))) uint32_t ctrl_stat_errors{};
__attribute__((section(".status.value"))) bool target_initialized{};
}

namespace tvsc::bringup {

template <typename ClockType>
tvsc::scheduler::Task<ClockType> flash_target(
    tvsc::hal::programmer::ProgrammerPeripheral &programmer_peripheral,
    tvsc::hal::gpio::GpioPeripheral &debug_led_peripheral, tvsc::hal::gpio::Pin debug_led_pin) {
  using namespace std::chrono_literals;
  using namespace tvsc::hal::gpio;
  using namespace tvsc::serial_wire;

  while (true) {
    tvsc::serial_wire::Result success{};
    {
      tvsc::serial_wire::SerialWire swd{programmer_peripheral};
      tvsc::serial_wire::Target target{swd};
      target_initialized = target.is_initialized();

      if (target_initialized) {
        tvsc::serial_wire::Flash flash{target};

        if (success) {
          success = target.initialize_ap_connection(0);
        }

        if (success) {
          success = target.ap_read_mem(tvsc::meta::BUILD_TIME_ADDR,
                                       reinterpret_cast<uint32_t *>(&target_build_time),
                                       sizeof(tvsc::meta::BuildTime) / sizeof(uint32_t));
        }

        if (success && target_build_time.timestamp < tvsc::meta::BUILD_TIME.timestamp) {
          auto debug_led{debug_led_peripheral.access()};

          debug_led.set_pin_mode(debug_led_pin, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

          for (int i = 0; i < 3; ++i) {
            debug_led.write_pin(debug_led_pin, 1);
            co_yield 250ms;
            debug_led.write_pin(debug_led_pin, 0);
            co_yield 250ms;
          }

          if (success) {
            success = target.halt();
          }

          if (success) {
            debug_led.write_pin(debug_led_pin, 1);
            success = flash.write_pages(FLASH_WRITE_START_PAGE, tvsc::meta::firmware_size_pages,
                                        tvsc::meta::firmware_start);
            debug_led.write_pin(debug_led_pin, 0);
          }

          (void)target.disable_debug();
          (void)target.power_off();
          swd.reset_target();

          if (success) {
            for (int i = 0; i < 5; ++i) {
              debug_led.write_pin(debug_led_pin, 1);
              co_yield 250ms;
              debug_led.write_pin(debug_led_pin, 0);
              co_yield 250ms;
            }
          } else {
            for (int i = 0; i < 5; ++i) {
              debug_led.write_pin(debug_led_pin, 1);
              co_yield 50ms;
              debug_led.write_pin(debug_led_pin, 0);
              co_yield 50ms;
            }
          }
        }
      }
    }

    co_yield 60s;
  }
}

}  // namespace tvsc::bringup
