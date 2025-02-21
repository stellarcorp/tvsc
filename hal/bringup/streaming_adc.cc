#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "hal/board/board.h"
#include "hal/bringup/dac_demo.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = BoardType::ClockType;
using TaskType = tvsc::hal::scheduler::Task<ClockType>;

extern "C" {

__attribute__((section(".status.value"))) uint32_t current_output_value{};
__attribute__((section(".status.value"))) std::array<uint32_t, 4> values_read{};
__attribute__((section(".status.value"))) volatile bool dma_complete{};
__attribute__((section(".status.value"))) volatile bool dma_error{};

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adc) { dma_complete = true; }

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* adc) { dma_error = true; }

}  // extern "C"

namespace tvsc::hal::bringup {

using namespace std::chrono_literals;
static constexpr std::chrono::microseconds PERIOD_US{500ms};

/**
 * NOTE: This bringup script is NOT fully functional.
 *
 * We still need to add support for internal timers and use a timer as the trigger for the ADC
 * conversion. We might also want to use the same timer, if possible, to trigger changing the DAC
 * value.
 */
template <uint8_t DAC_CHANNEL = 0>
TaskType run_adc_demo(BoardType& board) {
  auto& gpio_peripheral{board.gpio<BoardType::GREEN_LED_PORT>()};
  auto& adc_peripheral{board.adc()};
  auto& dac_peripheral{board.dac()};
  auto& timer_peripheral{board.timer2()};

  // Turn on clocks for the peripherals that we want.
  auto dac{dac_peripheral.access()};
  auto gpio{gpio_peripheral.access()};
  auto adc{adc_peripheral.access()};
  auto timer{timer_peripheral.access()};
  auto dac_out_gpio{board.gpio<BoardType::DAC_PORTS[DAC_CHANNEL]>().access()};

  dac_out_gpio.set_pin_mode(BoardType::DAC_PINS[DAC_CHANNEL], gpio::PinMode::ANALOG);
  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, gpio::PinMode::OUTPUT_PUSH_PULL, gpio::PinSpeed::LOW);

  static constexpr uint8_t RESOLUTION{8};
  static constexpr uint8_t RESOLUTION_SHIFT{RESOLUTION - 8};

  dac.set_resolution(RESOLUTION);
  adc.set_resolution(RESOLUTION);

  uint32_t iteration_counter{0};
  while (true) {
    // Recalibrate after a certain number of conversions.
    static constexpr uint32_t CALIBRATION_FREQUENCY{1024};
    if ((iteration_counter % CALIBRATION_FREQUENCY) == 0) {
      // Turn on LED while calibrating.
      gpio.write_pin(BoardType::GREEN_LED_PIN, 1);

      adc.calibrate_single_ended_input();

      gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
    }

    timer.start(PERIOD_US.count(), true);
    adc.start_conversion_stream({BoardType::DAC_CHANNEL_1_PORT, BoardType::DAC_CHANNEL_1_PIN},
                                values_read.data(), values_read.size() * 2, timer);

    dma_complete = false;
    dma_error = false;

    for (auto v : {256, 0, 1, 2, 4, 8, 16, 32, 64, 128}) {
      current_output_value = (v << RESOLUTION_SHIFT);
      dac.set_value(current_output_value);

      // Hold the DAC at this value. The ADC runs in the background and will measure this value
      // asynchronously.
      co_yield PERIOD_US;
    }

    // Stop the ADC.
    adc.reset_after_conversion();

    // Clear the state, DAC, and LED and pause between iterations.
    current_output_value = 0;
    dac.set_value(current_output_value);
    // Hold the DAC at this value.
    co_yield PERIOD_US;
    dac.clear_value();
    ++iteration_counter;
  }
}

}  // namespace tvsc::hal::bringup

using namespace tvsc::hal::bringup;
using namespace tvsc::hal::scheduler;

int main() {
  BoardType& board{BoardType::board()};

  Scheduler<ClockType, 4 /*QUEUE_SIZE*/> scheduler{board.clock(), board.rcc()};
  scheduler.add_task(run_adc_demo(board));
  scheduler.start();
}
