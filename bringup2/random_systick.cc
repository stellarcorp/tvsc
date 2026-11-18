#include <chrono>
#include <cmath>
#include <cstdint>
#include <random>

#include "base/initializer.h"
#include "hal2/board/board.h"
#include "hal2/config/device_registry.h"
#include "hal2/gpio/gpio.h"
#include "platforms/linker_sections.h"

using namespace std::chrono_literals;
using namespace tvsc::hal2;
using namespace tvsc::hal2::config;

static constexpr auto device_registry_builder{
    DeviceRegistryBuilder<tvsc::hal2::board::Board>{}  //
        .add<Facility::RNG_FACILITY, 0>()              //
        .add<Facility::CLOCK_FACILITY, 0>()            //
        .add<Facility::LED_FACILITY, 0>()              //
};
INTERRUPT_VECTOR_TABLE static constexpr auto interrupt_vector_table{
    device_registry_builder.create_vector_table()};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  auto device_registry{device_registry_builder.registry()};
  auto& clock{device_registry.steady_clock()};
  auto led{device_registry.debug_led().access()};

  led.set_pin_mode(tvsc::hal2::gpio::PinMode::OUTPUT_PUSH_PULL, tvsc::hal2::gpio::PinSpeed::LOW);
  led.off();

  auto rng{device_registry.random_device().access()};
  std::uniform_int_distribution<> distribution{100, 1500};

  while (true) {
    led.on();
    clock.sleep_ms(50);
    led.off();
    clock.sleep_ms(distribution(rng));
  }

  return 0;
}
