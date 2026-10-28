#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "hal2/board/board.h"
#include "hal2/config/system_builder.h"
#include "hal2/gpio/gpio.h"
#include "platforms/linker_sections.h"

using namespace std::chrono_literals;
using namespace tvsc::hal2::config;

int main(int argc, char* argv[]) {
  using PeripheralRegistry = tvsc::hal2::board::Board;
  static constexpr auto system_builder{
      SystemBuilder<PeripheralRegistry>{}         //
          .add<Facility::CLOCK_FACILITY, 0>()     //
          .add<Facility::WATCHDOG_FACILITY, 0>()  //
          .add<Facility::LED_FACILITY, 0>()       //
  };
  INTERRUPT_VECTOR_TABLE static constexpr auto interrupt_vector_table{
      system_builder.create_vector_table()};

  tvsc::initialize(&argc, &argv);
  auto sys{system_builder.system()};
  auto& clock{sys.steady_clock()};
  auto led{sys.debug_led().access()};

  // Initial sleep so that we can detect when the watchdog "bites" (causes a system reset).
  clock.sleep_ms(3'000);

  led.set_pin_mode(tvsc::hal2::gpio::PinMode::OUTPUT_PUSH_PULL, tvsc::hal2::gpio::PinSpeed::LOW);
  led.off();

  // Blink for a bit before we enable the watchdog.
  for (int i = 0; i < 5; ++i) {
    led.on();
    clock.sleep_ms(500);
    led.off();
    clock.sleep_ms(500);
  }

  // Turn on the watchdog but never feed it. This will result in a reset fairly shortly.
  auto watchdog(sys.watchdog0().access());

  // Loop until the watchdog bites. Also, change the style of the blinking so that we can track
  // progress through this script.
  while (true) {
    led.on();
    clock.sleep_ms(50);
    led.off();
    clock.sleep_ms(950);
  }

  return 0;
}
