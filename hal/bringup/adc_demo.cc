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
__attribute__((section(".status.value"))) std::array<uint32_t, 4> buffer{};
__attribute__((section(".status.value"))) float absolute_difference{};
__attribute__((section(".status.value"))) float relative_difference{};
__attribute__((section(".status.value"))) volatile bool dma_complete{};
__attribute__((section(".status.value"))) volatile bool dma_error{};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc) { dma_complete = true; }

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc) { dma_error = true; }
}

namespace tvsc::hal::bringup {

template <uint8_t DAC_CHANNEL = 0>
scheduler::Task run_adc_demo(BoardType& board) {
  auto& gpio_peripheral{board.gpio<BoardType::GREEN_LED_PORT>()};
  auto& adc_peripheral{board.adc()};
  auto& dac_peripheral{board.dac()};
  auto& clock{board.clock()};
  auto& dac_gpio_peripheral{board.gpio<BoardType::DAC_PORTS[DAC_CHANNEL]>()};

  // Turn on clocks for the peripherals that we want.
  auto dac{dac_peripheral.access()};
  auto gpio{gpio_peripheral.access()};
  auto adc{adc_peripheral.access()};
  auto dac_gpio{dac_gpio_peripheral.access()};

  dac_gpio.set_pin_mode(BoardType::DAC_PINS[DAC_CHANNEL], gpio::PinMode::ANALOG);

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

    for (auto v : {256, 0, 1, 2, 4, 8, 16, 32, 64, 128, 256}) {
      current_output_value = (v << RESOLUTION_SHIFT);
      dac.set_value(current_output_value);

      dma_complete = false;
      dma_error = false;

      // Set the values in the buffer to a known pattern to check for buffer overrun issues.
      buffer = {0xdead, 0xabcd, 0x1234, 0xef12};
      adc.start_single_conversion({BoardType::DAC_CHANNEL_1_PORT, BoardType::DAC_CHANNEL_1_PIN},
                                  buffer.data(), 1);
      while (!dma_complete) {
        // Yield while we take the measurement.
        co_yield 1000 * (5 + clock.current_time_millis());
      }
      adc.reset_after_conversion();

      uint16_t value_read = buffer[0];
      static constexpr float epsilon{0.01};
      relative_difference = std::abs(static_cast<float>(current_output_value) - value_read) /
                            (current_output_value + epsilon);
      absolute_difference = std::abs(static_cast<float>(current_output_value) - value_read);

      if (relative_difference < 0.25f || absolute_difference < (3 << RESOLUTION_SHIFT)) {
        // Success. Short solid.
        gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
        co_yield 1000 * (400 + clock.current_time_millis());
        gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
        co_yield 1000 * (100 + clock.current_time_millis());
      } else {
        // Failure. Flash frenetically.
        for (int i = 0; i < 2; ++i) {
          gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
          co_yield 1000 * (50 + clock.current_time_millis());
          gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
          co_yield 1000 * (50 + clock.current_time_millis());
        }
      }
    }
    ++iteration_counter;

    // Clear the state, DAC, and LED and pause between iterations.
    current_output_value = 0;
    absolute_difference = 0.f;
    relative_difference = 0.f;
    dac.set_value(current_output_value);
    dac.clear_value();
    gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
    co_yield 1000 * (500 + clock.current_time_millis());
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
