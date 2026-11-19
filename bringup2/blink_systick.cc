#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "hal2/board/board.h"
#include "hal2/devices/device_registry.h"
#include "hal2/gpio/gpio.h"
#include "platforms/linker_sections.h"

using namespace std::chrono_literals;
using namespace tvsc::hal2;
using namespace tvsc::hal2::devices;

static constexpr auto device_registry_builder{
    DeviceRegistryBuilder<tvsc::hal2::board::Board>{}
        .add<Facility::CLOCK_FACILITY, 0, {.sleep_strategy = clock::SleepStrategy::STOP}>()
        .add<Facility::LED_FACILITY, 0>()};

INTERRUPT_VECTOR_TABLE static constexpr auto interrupt_vector_table{
    device_registry_builder.create_vector_table()};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  auto device_registry{device_registry_builder.registry()};
  auto& clock{device_registry.steady_clock()};
  auto led{device_registry.debug_led().access()};

  led.set_pin_mode(gpio::PinMode::OUTPUT_PUSH_PULL, gpio::PinSpeed::LOW);
  led.off();

  while (true) {
    led.on();
    clock.sleep(50ms);
    led.off();
    clock.sleep(950ms);
  }

  return 0;
}
