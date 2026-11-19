#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "hal2/board/board.h"
#include "hal2/registry/device_registry.h"
#include "hal2/gpio/gpio.h"
#include "platforms/linker_sections.h"

using namespace std::chrono_literals;
using namespace tvsc::hal2;
using namespace tvsc::hal2::registry;

static constexpr auto device_registry_builder{
    DeviceRegistryBuilder<tvsc::hal2::board::Board>{}                                        //
        .add<Facility::CLOCK_FACILITY, 0, {.sleep_strategy = clock::SleepStrategy::STOP}>()  //
        .add<Facility::WATCHDOG_FACILITY, 0>()                                               //
        .add<Facility::LED_FACILITY, 0>()                                                    //
};
INTERRUPT_VECTOR_TABLE static constexpr auto interrupt_vector_table{
    device_registry_builder.create_vector_table()};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  auto device_registry{device_registry_builder.registry()};
  auto& clock{device_registry.steady_clock()};
  auto led{device_registry.debug_led().access()};

  // Initial sleep so that we can detect when the watchdog "bites" (causes a system reset).
  clock.sleep_ms(3'000);

  led.set_pin_mode(gpio::PinMode::OUTPUT_PUSH_PULL, gpio::PinSpeed::LOW);
  led.off();

  // Blink for a bit before we enable the watchdog.
  for (int i = 0; i < 5; ++i) {
    led.on();
    clock.sleep_ms(500);
    led.off();
    clock.sleep_ms(500);
  }

  // Turn on the watchdog but never feed it. This will result in a reset fairly shortly.
  auto watchdog(device_registry.watchdog0().access());

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
