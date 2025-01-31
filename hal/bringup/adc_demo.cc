#include <cmath>
#include <cstdint>
#include <cstring>

#include "hal/board/board.h"
#include "hal/bringup/dac_demo.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"

using BoardType = tvsc::hal::board::Board;

__attribute__((section(".status.value"))) uint32_t current_output_dac;
__attribute__((section(".status.value"))) uint32_t value_read;
__attribute__((section(".status.value"))) float absolute_difference;
__attribute__((section(".status.value"))) float relative_difference;

namespace tvsc::hal::bringup {

scheduler::Task run_adc(BoardType& board, uint32_t& value_read) {
  // Turn on clocks for the peripherals that we want.
  board.rcc().enable_adc_clock();
  board.rcc().enable_dma_clock();

  uint32_t iteration_counter{0};
  while (true) {
    auto& adc{board.adc()};
    auto& clock{board.clock()};

    // Recalibrate after a certain number of conversions.
    static constexpr uint32_t CALIBRATION_FREQUENCY{1024};
    if (iteration_counter % CALIBRATION_FREQUENCY == 0) {
      adc.calibrate_single_ended_input();
      while (adc.is_running()) {
        // Yield while we calibrate.
        co_yield 1000 * (5 + clock.current_time_millis());
      }
    }

    adc.start_conversion({BoardType::DAC_CHANNEL_1_PORT, BoardType::DAC_CHANNEL_1_PIN}, &value_read,
                         1);
    while (adc.is_running()) {
      // Yield while we take the measurement.
      co_yield 1000 * (5 + clock.current_time_millis());
    }

    ++iteration_counter;
  }
}

scheduler::Task compare_values(BoardType& board, uint32_t& expected, uint32_t& actual) {
  // Turn on clocks for the peripherals that we want.
  board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);

  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, gpio::PinMode::OUTPUT_PUSH_PULL, gpio::PinSpeed::LOW);

  while (true) {
    auto& clock{board.clock()};

    static constexpr float epsilon{0.01};
    relative_difference = std::abs(static_cast<float>(actual) - expected) / (expected + epsilon);
    absolute_difference = std::abs(static_cast<float>(actual) - expected);

    if (relative_difference < 0.25f) {
      // Success. Short solid.
      gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
      co_yield 1000 * (250 + clock.current_time_millis());
      gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
      co_yield 1000 * (50 + clock.current_time_millis());
    } else {
      // Failure. Flash frenetically.
      for (int i = 0; i < 5; ++i) {
        gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
        co_yield 1000 * (50 + clock.current_time_millis());
        gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
        co_yield 1000 * (50 + clock.current_time_millis());
      }
    }

    absolute_difference = 0.f;
    relative_difference = 0.f;
    gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
    co_yield 1000 * (500 + clock.current_time_millis());
  }
}

}  // namespace tvsc::hal::bringup

using namespace tvsc::hal::bringup;
using namespace tvsc::hal::scheduler;

int main() {
  BoardType board{};

  Scheduler<4 /*QUEUE_SIZE*/> scheduler{board.clock()};
  scheduler.add_task(run_dac_demo<0>(board, current_output_dac));
  scheduler.add_task(run_adc(board, value_read));
  scheduler.add_task(compare_values(board, current_output_dac, value_read));
  scheduler.start();
}
