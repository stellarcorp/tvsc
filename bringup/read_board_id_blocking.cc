#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/board_identification/board_ids.h"
#include "hal/gpio/gpio.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;
using namespace tvsc::hal::board_identification;
using namespace tvsc::hal::gpio;
using namespace std::chrono_literals;

extern "C" {
alignas(uint32_t)  //
    __attribute__((section(".status.value"))) volatile BoardId board_id{};
}

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType& board{BoardType::board()};

  auto& gpio_id_power_peripheral{board.gpio<BoardType::BOARD_ID_POWER_PORT>()};
  auto& gpio_id_sense_peripheral{board.gpio<BoardType::BOARD_ID_SENSE_PORT>()};
  auto& adc_peripheral{board.adc()};

  // Turn on clocks for the peripherals that we want.
  auto gpio_id_power{gpio_id_power_peripheral.access()};
  auto gpio_id_sense{gpio_id_sense_peripheral.access()};
  auto adc{adc_peripheral.access()};

  adc.calibrate_single_ended_input();

  adc.set_resolution(tvsc::hal::board_identification::BOARD_ID_ADC_RESOLUTION_BITS);
  if constexpr (tvsc::hal::board_identification::BOARD_ID_ADC_RESOLUTION_BITS < 8) {
    adc.use_data_align_left();
  }

  gpio_id_power.set_pin_mode(BoardType::BOARD_ID_POWER_PIN, PinMode::OUTPUT_PUSH_PULL,
                             PinSpeed::LOW);

  gpio_id_sense.set_pin_mode(BoardType::BOARD_ID_SENSE_PIN, PinMode::ANALOG);

  while (true) {
    gpio_id_power.write_pin(BoardType::BOARD_ID_POWER_PIN, 1);

    const auto measured_value{
        adc.measure_value({BoardType::BOARD_ID_SENSE_PORT, BoardType::BOARD_ID_SENSE_PIN})};

    gpio_id_power.write_pin(BoardType::BOARD_ID_POWER_PIN, 0);

    board_id = determine_board_id(measured_value);

    ClockType::clock().wait(10ms);
  }
}
