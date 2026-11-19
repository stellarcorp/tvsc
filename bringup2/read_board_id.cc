#include <chrono>
#include <cstdint>

#include "base/enums.h"
#include "base/initializer.h"
#include "hal2/board/board.h"
#include "hal2/board_identification/board_ids.h"
#include "hal2/devices/device_registry.h"
#include "hal2/gpio/gpio.h"
#include "platforms/linker_sections.h"

using namespace std::chrono_literals;
using namespace tvsc;
using namespace tvsc::hal2;
using namespace tvsc::hal2::board_identification;
using namespace tvsc::hal2::devices;

DEBUG_VAR BoardId board_id{};

static constexpr auto device_registry_builder{
    DeviceRegistryBuilder<board::Board>{}
        .add<Facility::BOARD_IDENTIFICATION_FACILITY, 0>()
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

  uint32_t iteration_count{};
  while (true) {
    static constexpr uint32_t CALIBRATION_FREQUENCY{1024};
    if (iteration_count++ % CALIBRATION_FREQUENCY == 0) {
      auto adc{device_registry.adc0().access()};
      adc.calibrate_single_ended_input();
    }

    {
      auto board_identifier{device_registry.board_id().access()};
      board_id = board_identifier.read_board_id(clock);
    }
    if (board_id == cast_to_underlying_type(CanonicalBoardIds::UNKNOWN)) {
      for (int i = 0; i < 10; ++i) {
        led.on();
        clock.sleep(50ms);
        led.off();
        clock.sleep(100ms);
      }
    } else {
      for (int i = 0; i < 5; ++i) {
        led.on();
        clock.sleep(50ms);
        led.off();
        clock.sleep(950ms);
      }
    }
    led.off();
    clock.sleep(2s);
  }

  return 0;
}
