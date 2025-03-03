#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "bringup/dac_demo.h"
#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/embedded_clock.h"

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

namespace tvsc::bringup {

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::hal::time::EmbeddedClock;

template <typename ClockType, uint8_t DAC_CHANNEL = 0>
tvsc::hal::scheduler::Task<ClockType> run_adc_demo(BoardType& board) {
  using namespace std::chrono_literals;
  auto& gpio_peripheral{board.gpio<BoardType::GREEN_LED_PORT>()};
  auto& adc_peripheral{board.adc()};
  auto& dac_peripheral{board.dac()};
  auto& dac_gpio_peripheral{board.gpio<BoardType::DAC_PORTS[DAC_CHANNEL]>()};

  // Turn on clocks for the peripherals that we want.
  auto dac{dac_peripheral.access()};
  auto gpio{gpio_peripheral.access()};
  auto adc{adc_peripheral.access()};
  auto dac_gpio{dac_gpio_peripheral.access()};

  dac_gpio.set_pin_mode(BoardType::DAC_PINS[DAC_CHANNEL], tvsc::hal::gpio::PinMode::ANALOG);

  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL, tvsc::hal::gpio::PinSpeed::LOW);

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
      co_yield 500ms;
      gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
      co_yield 500ms;
      gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
      co_yield 500ms;
      gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
      co_yield 500ms;
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
        co_yield 5ms;
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
        co_yield 400ms;
        gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
        co_yield 100ms;
      } else {
        // Failure. Flash frenetically.
        for (int i = 0; i < 2; ++i) {
          gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
          co_yield 50ms;
          gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
          co_yield 50ms;
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
    co_yield 500ms;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::hal::scheduler;

int main() {
  BoardType& board{BoardType::board()};

  Scheduler<ClockType, 4 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(run_adc_demo<ClockType>(board));
  scheduler.start();
}
