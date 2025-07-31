#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include "base/enums.h"
#include "base/initializer.h"
#include "bits/bits.h"
#include "bringup/blink.h"
#include "hal/board/board.h"
#include "hal/board_identification/board_ids.h"
#include "hal/gpio/gpio.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

extern "C" {

alignas(uint32_t)  //
    __attribute__((
        section(".status.value"))) volatile tvsc::hal::board_identification::BoardId board_id{};
__attribute__((section(".status.value"))) std::array<uint32_t, 1> buffer{};
__attribute__((section(".status.value"))) volatile bool dma_complete{};
__attribute__((section(".status.value"))) volatile bool dma_error{};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc) { dma_complete = true; }

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc) { dma_error = true; }
}

namespace tvsc::bringup {

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

template <typename ClockType>
tvsc::scheduler::Task<ClockType> read_board_id(BoardType& board, int32_t num_iterations = -1) {
  using namespace std::chrono_literals;
  auto& gpio_id_power_peripheral{board.gpio<BoardType::BOARD_ID_POWER_PORT>()};
  auto& gpio_id_sense_peripheral{board.gpio<BoardType::BOARD_ID_SENSE_PORT>()};
  auto& adc_peripheral{board.adc()};

  // Turn on clocks for the peripherals that we want.
  auto gpio_id_power{gpio_id_power_peripheral.access()};
  auto gpio_id_sense{gpio_id_sense_peripheral.access()};
  auto adc{adc_peripheral.access()};

  gpio_id_power.set_pin_mode(BoardType::BOARD_ID_POWER_PIN,
                             tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL,
                             tvsc::hal::gpio::PinSpeed::LOW);

  gpio_id_sense.set_pin_mode(BoardType::BOARD_ID_SENSE_PIN, tvsc::hal::gpio::PinMode::ANALOG);

  int32_t iteration_counter{0};
  while (num_iterations < 0 || iteration_counter < num_iterations) {
    // Recalibrate after a certain number of conversions.
    static constexpr uint32_t CALIBRATION_FREQUENCY{1024};
    if ((iteration_counter % CALIBRATION_FREQUENCY) == 0) {
      adc.calibrate_single_ended_input();
    }

    adc.set_resolution(tvsc::hal::board_identification::BOARD_ID_ADC_RESOLUTION_BITS);
    if constexpr (tvsc::hal::board_identification::BOARD_ID_ADC_RESOLUTION_BITS < 8) {
      adc.use_data_align_left();
    }

    // Turn on the power to the board id circuitry.
    gpio_id_power.write_pin(BoardType::BOARD_ID_POWER_PIN, 1);
    // Let the circuit settle.
    co_yield 1ms;

    // Set the values in the buffer to a known pattern to check for buffer overrun issues.
    for (size_t i = 0; i < buffer.size(); ++i) {
      buffer[i] = 0xfefefefe;
    }

    dma_complete = false;
    dma_error = false;

    adc.start_single_conversion({BoardType::BOARD_ID_SENSE_PORT, BoardType::BOARD_ID_SENSE_PIN},
                                buffer.data(), buffer.size());

    while (!dma_complete) {
      // Yield while we take the measurement.
      co_yield 5ms;
    }
    adc.reset_after_conversion();

    // Turn off the power to the board id circuitry.
    gpio_id_power.write_pin(BoardType::BOARD_ID_POWER_PIN, 0);

    uint16_t value_read = buffer[0];

    board_id = tvsc::hal::board_identification::determine_board_id(value_read);

    // Pause between iterations.
    co_yield 50ms;
    ++iteration_counter;
  }

  // Turn off the power to the board id circuitry.
  gpio_id_power.write_pin(BoardType::BOARD_ID_POWER_PIN, 0);
  co_return;
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType& board{BoardType::board()};

  Scheduler<ClockType, 2 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(read_board_id<ClockType>(board));
  scheduler.add_task(blink_on_success<ClockType>(
      []() {
        return board_id != tvsc::cast_to_underlying_type(
                               tvsc::hal::board_identification::CanonicalBoardIds::UNKNOWN);
      },
      board.gpio<BoardType::DEBUG_LED_PORT>(), board.DEBUG_LED_PIN));
  scheduler.start();
}
