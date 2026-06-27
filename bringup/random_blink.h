#include <chrono>

#include "hal/gpio/gpio.h"
#include "random/random.h"
#include "system/system.h"

namespace tvsc::bringup {

tvsc::system::System::Task blink_randomly(tvsc::hal::gpio::PinPeripheral& led_peripheral) {
  auto led{led_peripheral.access()};
  led.set_pin_mode(tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);
  led.write_pin(/* OFF */ 0);

  while (true) {
    led.toggle_pin();
    const std::chrono::milliseconds delay_ms{tvsc::random::generate_random_value(5U, 1000U)};
    co_yield delay_ms;
  }
}

}  // namespace tvsc::bringup
