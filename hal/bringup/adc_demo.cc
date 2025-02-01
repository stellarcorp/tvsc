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

__attribute__((section(".status.value"))) uint32_t current_output_value;
__attribute__((section(".status.value"))) uint32_t value_read;
__attribute__((section(".status.value"))) float absolute_difference;
__attribute__((section(".status.value"))) float relative_difference;
__attribute__((section(".status.value"))) volatile bool dma_complete;
__attribute__((section(".status.value"))) volatile bool dma_half_complete;
__attribute__((section(".status.value"))) volatile bool dma_error;
__attribute__((section(".status.value"))) volatile uint8_t just_checking_we_can_write_memory;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc) { dma_complete = true; }

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* adc) { dma_half_complete = true; }

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc) { dma_error = true; }
}

namespace tvsc::hal::bringup {

template <uint8_t DAC_CHANNEL = 0>
scheduler::Task run_adc_demo(BoardType& board) {
  // Turn on clocks for the peripherals that we want.
  board.rcc().enable_adc_clock();
  board.rcc().enable_dma_clock();
  board.rcc().enable_dac_clock();
  board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);

  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, gpio::PinMode::OUTPUT_PUSH_PULL, gpio::PinSpeed::LOW);

  {
    auto& dac_out_gpio{board.gpio<BoardType::DAC_PORTS[DAC_CHANNEL]>()};
    dac_out_gpio.set_pin_mode(BoardType::DAC_PINS[DAC_CHANNEL], gpio::PinMode::ANALOG);
  }

  auto& adc{board.adc()};
  auto& dac{board.dac()};
  auto& clock{board.clock()};

  dac.set_resolution(8);

  uint32_t iteration_counter{0};
  while (true) {
    // Recalibrate after a certain number of conversions.
    static constexpr uint32_t CALIBRATION_FREQUENCY{1024};
    if ((iteration_counter % CALIBRATION_FREQUENCY) == 0) {
      // adc.calibrate_single_ended_input();
      // while (adc.is_running()) {
      //   gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
      //   // Yield while we calibrate.
      //   co_yield 1000 * (5 + clock.current_time_millis());
      // }

      // Flash after calibration.
      gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
      co_yield 1000 * (50 + clock.current_time_millis());
      gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
      co_yield 1000 * (50 + clock.current_time_millis());
      gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
      co_yield 1000 * (50 + clock.current_time_millis());
      gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
      co_yield 1000 * (50 + clock.current_time_millis());
    }

    for (auto v : {256, 0, 1, 2, 4, 8, 16, 32, 64, 128, 256}) {
      current_output_value = v;
      dac.set_value(current_output_value);

      // Let the DAC settle.
      co_yield 1'000 + clock.current_time_micros();

      dma_complete = false;
      dma_half_complete = false;
      dma_error = false;

      // Just write some value that let's us know our progress.
      just_checking_we_can_write_memory = 0xab;

      adc.start_conversion({BoardType::DAC_CHANNEL_1_PORT, BoardType::DAC_CHANNEL_1_PIN},
                           &value_read, 1);
      while (!dma_complete) {
        // Yield while we take the measurement.
        co_yield 1000 * (5 + clock.current_time_millis());

        // Blink the lights so we know what is happening.
        gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
        co_yield 1000 * (200 + clock.current_time_millis());
        gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
        co_yield 1000 * (200 + clock.current_time_millis());
      }

      static constexpr float epsilon{0.01};
      relative_difference = std::abs(static_cast<float>(current_output_value) - value_read) /
                            (current_output_value + epsilon);
      absolute_difference = std::abs(static_cast<float>(current_output_value) - value_read);

      if (relative_difference < 0.25f) {
        // Success. Short solid.
        gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
        co_yield 1000 * (250 + clock.current_time_millis());
        gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
        co_yield 1000 * (50 + clock.current_time_millis());
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
    value_read = 0;
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
