#include <chrono>
#include <cstdint>
#include <limits>

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
      SystemBuilder<PeripheralRegistry>{}        //
          .add<Facility::RCC_FACILITY, 0>()      //
          .add<Facility::SYSTICK_FACILITY, 0>()  //
          .add<Facility::TIMER_FACILITY, 1>()    //
          .add<Facility::CLOCK_FACILITY, 0>()    //
          .add<Facility::LED_FACILITY, 0>()      //
  };
  static constexpr size_t ISR_ARRAY_SIZE{100};
  INTERRUPT_VECTOR_TABLE static constexpr auto interrupt_vector_table{
      system_builder.create_vector_table<ISR_ARRAY_SIZE>()};

  tvsc::initialize(&argc, &argv);
  auto sys{system_builder.system()};
  auto& clock{sys.steady_clock()};
  auto led{sys.debug_led().access()};

  led.set_pin_mode(tvsc::hal2::gpio::PinMode::OUTPUT_PUSH_PULL, tvsc::hal2::gpio::PinSpeed::LOW);
  led.off();

  while (true) {
    led.toggle_pin();
    clock.sleep_us(500'000);
  }

  return 0;
}
