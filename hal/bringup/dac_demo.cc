#include <cstring>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"

using BoardType = tvsc::hal::board::Board;

using namespace tvsc::hal::gpio;

int main() {
  BoardType board{};

  // Turn on clocks for the peripherals that we want.
  board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);
  board.rcc().enable_dac_clock();

  {
    auto& dac_out_gpio{board.gpio<BoardType::DAC_PORT>()};
    dac_out_gpio.set_pin_mode(BoardType::DAC_PIN, PinMode::ANALOG);
  }

  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  uint8_t dac_values[] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 255};
  while (true) {
    auto& dac{board.dac()};
    auto& clock{board.clock()};

    for (const auto& v : dac_values) {
      dac.set_value(v);
      gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
      clock.sleep_ms(100);
      gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
      clock.sleep_ms(400);
    }

    dac.clear_value();
    gpio.write_pin(BoardType::GREEN_LED_PIN, 1);
    clock.sleep_ms(500);
    gpio.write_pin(BoardType::GREEN_LED_PIN, 0);
    clock.sleep_ms(500);
  }
}
