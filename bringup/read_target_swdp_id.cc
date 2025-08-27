#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "bits/bits.h"
#include "hal/gpio/gpio.h"
#include "hal/programmer/programmer.h"
#include "serial_wire/serial_wire.h"
#include "serial_wire/target.h"
#include "system/system.h"

extern "C" {
__attribute__((section(".status.value"))) uint32_t target_swdp_id{};
}

namespace tvsc::bringup {

tvsc::system::System::Task read_target_swdp_id() {
  using BoardType = tvsc::system::System::BoardType;
  using namespace std::chrono_literals;
  using namespace tvsc::hal::gpio;
  using namespace tvsc::serial_wire;
  using namespace tvsc::system;

  static constexpr uint32_t EXPECTED_SW_DP_IDCODE{0x2BA01477};

  auto &debug_led_peripheral{System::board().gpio<BoardType::DEBUG_LED_PORT>()};
  auto &programmer_peripheral{System::board().programmer()};

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
using namespace tvsc::system;

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);

  System::scheduler().add_task(read_target_swdp_id());
  System::scheduler().start();
}
