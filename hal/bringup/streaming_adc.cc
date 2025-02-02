#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "hal/board/board.h"
#include "hal/bringup/dac_demo.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"

using BoardType = tvsc::hal::board::Board;

extern "C" {

__attribute__((section(".status.value"))) uint32_t current_output_value{};
__attribute__((section(".status.value"))) std::array<uint32_t, 16> values_read{};
__attribute__((section(".status.value"))) volatile bool dma_complete{};
__attribute__((section(".status.value"))) volatile bool dma_error{};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc) { dma_complete = true; }

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc) { dma_error = true; }
}

namespace tvsc::hal::bringup {

/**
 * NOTE: This bringup script is NOT fully functional.
 *
 * We still need to add support for internal timers and use a timer as the trigger for the ADC
 * conversion. We might also want to use the same timer, if possible, to trigger changing the DAC
 * value.
 */
template <uint8_t DAC_CHANNEL = 0>
scheduler::Task run_adc_demo(BoardType& board) {
  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  auto& adc{board.adc()};
  auto& dac{board.dac()};
  auto& clock{board.clock()};

  // Turn on clocks for the peripherals that we want.
  const EnableLock dac_power{dac.enable()};
  const EnableLock gpio_power{gpio.enable()};
  const EnableLock dma_power{board.dma().enable()};
  const EnableLock adc_power{adc.enable()};

  {
    auto& dac_out_gpio{board.gpio<BoardType::DAC_PORTS[DAC_CHANNEL]>()};
    dac_out_gpio.set_pin_mode(BoardType::DAC_PINS[DAC_CHANNEL], gpio::PinMode::ANALOG);
  }

  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, gpio::PinMode::OUTPUT_PUSH_PULL, gpio::PinSpeed::LOW);

  static constexpr uint8_t RESOLUTION{12};
  static constexpr uint8_t RESOLUTION_SHIFT{RESOLUTION - 8};

  dac.set_resolution(RESOLUTION);
  adc.set_resolution(RESOLUTION);

  uint32_t iteration_counter{0};
  while (true) {
    // Recalibrate after a certain number of conversions.
    static constexpr uint32_t CALIBRATION_FREQUENCY{16};
    if ((iteration_counter % CALIBRATION_FREQUENCY) == 0) {
      adc.calibrate_single_ended_input();

      // Flash slowly after calibration.
      gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
      co_yield 1000 * (500 + clock.current_time_millis());
      gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
      co_yield 1000 * (500 + clock.current_time_millis());
      gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
      co_yield 1000 * (500 + clock.current_time_millis());
      gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
      co_yield 1000 * (500 + clock.current_time_millis());
    }

    current_output_value = (128 << RESOLUTION_SHIFT);
    dac.set_value(current_output_value);

    // TODO(james): Need to add a timer here. As it is, the ADC just runs values_read.size()
    // conversions in a row, very quickly, resulting in the entire array getting filled with the
    // first value in the array of the for-loop below.
    adc.start_conversion_stream({BoardType::DAC_CHANNEL_1_PORT, BoardType::DAC_CHANNEL_1_PIN},
                                values_read.data(), values_read.size());
    dma_complete = false;
    dma_error = false;

    for (auto v : {256, 0, 1, 2, 4, 8, 16, 32, 64, 128, 256}) {
      current_output_value = (v << RESOLUTION_SHIFT);
      dac.set_value(current_output_value);

      // Hold the DAC at this value for a while.
      co_yield 1'000'000 + clock.current_time_micros();
    }

    // Stop the ADC.
    adc.reset_after_conversion();

    // Clear the state, DAC, and LED and pause between iterations.
    current_output_value = 0;
    dac.set_value(current_output_value);
    dac.clear_value();
    ++iteration_counter;

    // Flash after conversion.
    gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
    co_yield 1000 * (250 + clock.current_time_millis());
    gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
    co_yield 1000 * (100 + clock.current_time_millis());
    gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
    co_yield 1000 * (250 + clock.current_time_millis());
    gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
    co_yield 1000 * (100 + clock.current_time_millis());
  }
}

}  // namespace tvsc::hal::bringup

using namespace tvsc::hal::bringup;
using namespace tvsc::hal::scheduler;

int main() {
  BoardType& board{BoardType::board()};

  Scheduler<4 /*QUEUE_SIZE*/> scheduler{board.clock()};
  scheduler.add_task(run_adc_demo(board));
  scheduler.start();
}
