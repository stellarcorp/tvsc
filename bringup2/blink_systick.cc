#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "hal2/board/board.h"
#include "hal2/config/system_builder.h"
#include "hal2/gpio/gpio.h"
#include "platforms/linker_sections.h"

using namespace std::chrono_literals;
using namespace tvsc::hal2;
using namespace tvsc::hal2::config;

static constexpr auto system_builder{
    SystemBuilder<tvsc::hal2::board::Board>{}
        .add<Facility::CLOCK_FACILITY, 0, {.sleep_strategy = clock::SleepStrategy::STOP}>()
        .add<Facility::LED_FACILITY, 0>()};

INTERRUPT_VECTOR_TABLE static constexpr auto interrupt_vector_table{
    system_builder.create_vector_table()};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);
  auto sys{system_builder.system()};
  auto& clock{sys.steady_clock()};
  auto led{sys.debug_led().access()};

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
