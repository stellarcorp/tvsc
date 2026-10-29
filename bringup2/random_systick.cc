#include <chrono>
#include <cstdint>
#include <random>

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
      SystemBuilder<PeripheralRegistry>{}      //
          .add<Facility::RNG_FACILITY, 0>()    //
          .add<Facility::CLOCK_FACILITY, 0>()  //
          .add<Facility::LED_FACILITY, 0>()    //
  };
  INTERRUPT_VECTOR_TABLE static constexpr auto interrupt_vector_table{
      system_builder.create_vector_table()};

  tvsc::initialize(&argc, &argv);
  auto sys{system_builder.system()};
  auto& clock{sys.steady_clock()};
  auto led{sys.debug_led().access()};

  led.set_pin_mode(tvsc::hal2::gpio::PinMode::OUTPUT_PUSH_PULL, tvsc::hal2::gpio::PinSpeed::LOW);
  led.off();

  auto rng{sys.random_device().access()};
  std::uniform_int_distribution<> distribution{100, 1500};

  while (true) {
    led.on();
    clock.sleep_ms(50);
    led.off();
    clock.sleep_ms(distribution(rng));
  }

  return 0;
}
