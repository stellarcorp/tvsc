#include <chrono>

#include "hal/led/led.h"
#include "random/random.h"
#include "system/system.h"

namespace tvsc::bringup {

tvsc::system::System::Task blink_randomly(tvsc::hal::led::LedPeripheral& led_peripheral) {
  auto led{led_peripheral.access()};
  led.off();

  while (true) {
    led.toggle();
    const std::chrono::milliseconds delay_ms{tvsc::random::generate_random_value(5U, 1000U)};
    co_yield delay_ms;
  }
}

}  // namespace tvsc::bringup
