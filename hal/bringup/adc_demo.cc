#include <cmath>
#include <cstdint>
#include <cstring>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"

using BoardType = tvsc::hal::board::Board;

__attribute__((section(".status.value"))) volatile uint32_t value_expected;
__attribute__((section(".status.value"))) volatile uint32_t value_read;
__attribute__((section(".status.value"))) volatile float absolute_difference;
__attribute__((section(".status.value"))) volatile float relative_difference;

using namespace tvsc::hal::gpio;

int main() {
  BoardType board{};

  // Turn on clocks for the peripherals that we want.
  board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);
  board.rcc().enable_dac_clock();
  board.rcc().enable_adc_clock();

  {
    auto& dac_out_gpio{board.gpio<BoardType::DAC_CHANNEL_1_PORT>()};
    dac_out_gpio.set_pin_mode(BoardType::DAC_CHANNEL_1_PIN, PinMode::ANALOG);
  }

  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  uint8_t dac_values[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 255};
  while (true) {
    auto& adc{board.adc()};
    auto& dac{board.dac()};
    auto& clock{board.clock()};

    adc.calibrate_single_ended_input();
    while (adc.is_running()) {
      // Do nothing.
    }

    dac.set_resolution(8);
    for (const auto& v : dac_values) {
      dac.set_value(v);
      value_expected = v;

      clock.sleep_ms(10);
      adc.start_conversion(BoardType::DAC_CHANNEL_1_PORT, BoardType::DAC_CHANNEL_1_PIN);
      while (adc.is_running()) {
        // Block while we take the measurement.
      }
      value_read = adc.read_result();

      static constexpr float epsilon{0.01};
      relative_difference =
          std::abs(static_cast<float>(value_read) - value_expected) / (value_expected + epsilon);
      absolute_difference = std::abs(static_cast<float>(value_read) - value_expected);

      if (relative_difference < 0.25f) {
        // Success. Short solid.
        gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
        clock.sleep_ms(250);
        gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
        clock.sleep_ms(50);
      } else {
        // Failure. Flash frenetically.
        for (int i = 0; i < 5; ++i) {
          gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
          clock.sleep_ms(50);
          gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
          clock.sleep_ms(50);
        }
      }
    }

    value_expected = 0;
    value_read = 0;
    absolute_difference = 0.f;
    relative_difference = 0.f;
    dac.clear_value();
    gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
    clock.sleep_ms(500);
    gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
    clock.sleep_ms(500);
  }
}
